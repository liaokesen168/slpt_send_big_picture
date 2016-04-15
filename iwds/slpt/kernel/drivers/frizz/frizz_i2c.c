#include <i2c.h>
#include <common.h>
#include "frizz_i2c.h"
#include "frizz_reg.h"
#include "frizz_debug.h"
#include "frizz_interface.h"

//#define MAX_ACCESS_LIMIT  	(1024*8)		/*!< max transfer size         */
#define MAX_ACCESS_LIMIT  	(32)		/*!< max transfer size         */

#define IGNORE_CTRL_RESET_RETURN (1)

#define FRIZZ_VIO_MIN_UV	1750000
#define FRIZZ_VIO_MAX_UV	1950000

static int i2c_master_send(unsigned int reg_addr, unsigned char *buff, int buff_size) {
	int status = -1;
	status = i2c_write(I2C_FRIZZ_SLAVE_ADDRESS, reg_addr, 1, buff, buff_size);
	return status;
}
static int i2c_master_recv(unsigned int reg_addr, unsigned char *buff, int buff_size) {
	int status = -1;
	status = i2c_read(I2C_FRIZZ_SLAVE_ADDRESS, reg_addr, 1, buff, buff_size);
	return status;
}

static int __frizz_i2c_read(unsigned int reg_addr, unsigned char *rx_buff, int buff_size) 
{
	int status = -1;
	status = i2c_master_recv(reg_addr, rx_buff, buff_size);
  	return status;
}

static int __frizz_i2c_write(unsigned char *data, int data_size) 
{
	int status = i2c_master_send(data[1], data + 1, data_size - 1);

	if(0 <= status){
		status = 0;
	}
#ifdef IGNORE_CTRL_RESET_RETURN
	else if(data_size == 5 && data[0]==0 && data[1]==0 && data[2]==0 && data[3]==0 && data[4]==1){

		DEBUG_PRINT("frizz: __frizz_i2c_write() ignore ctrl reset\n" );
		status = 0;
	}
#endif		
	else{
		DEBUG_PRINT("frizz: __frizz_i2c_write() send error (%d)\n", status );
	}

	return status;
}

static int __i2c_write_reg_32(unsigned int reg_addr, unsigned int data)
{
	unsigned char tx_buff[5];
	int status = 0;
	
	tx_buff[0] = (unsigned char)reg_addr;
	tx_buff[1] = (data >> 24) & 0xff;
	tx_buff[2] = (data >> 16) & 0xff;
	tx_buff[3] = (data >>  8) & 0xff;
	tx_buff[4] = (data >>  0) & 0xff;

	status = __frizz_i2c_write(tx_buff, sizeof(tx_buff));

	if (0 <= status ){
		status = 0;
	}
	else{
		DEBUG_PRINT("frizz: __i2c_write_reg_32(reg:%#x, data:%d) => %d\n", reg_addr, data, status );
	}
	return status;
}

int i2c_write_reg_32(unsigned int reg_addr, unsigned int data)
{
		int status = __i2c_write_reg_32(reg_addr, data);

		if(0 <= status ){
			status = 0;
		}
		else{
			DEBUG_PRINT("frizz: i2c_write_reg_32(reg:%#x, data:%d) => %d\n", reg_addr, data, status );
		}
		return status;
}

static int __i2c_read_reg_32(unsigned int reg_addr, unsigned int* data)
{
	unsigned char rx_buff[4];

	int status = __frizz_i2c_read(reg_addr, rx_buff, sizeof(rx_buff));

	if (0 <= status) {
		*data = CONVERT_UINT(rx_buff[0], rx_buff[1] , rx_buff[2] , rx_buff[3] );
		status = 0;
	}
	else{
		DEBUG_PRINT("frizz: __i2c_read_reg_32(reg:%#x) => %d\n", reg_addr, status );
	}
	return status;
}

int i2c_read_reg_32(unsigned int reg_addr, unsigned int* data)
{
	int status = __i2c_read_reg_32(reg_addr, data);

	if(status ){
		DEBUG_PRINT("frizz: i2c_read_reg_32(reg:%#x) => %d\n", reg_addr, status );
	}
	return status;
}

static int __i2c_read_reg_array(unsigned int reg_addr, unsigned char *array, int array_size) 
{
	int status=0;

	status = __frizz_i2c_read(reg_addr, array, array_size);

	status = (0 < status ? 0 : status);

	return status;
}

int i2c_read_reg_array(unsigned int reg_addr, unsigned char *array, int array_size) 
{
	int status=0;

	{
		int rest_size = array_size;
		unsigned char* p = array;
		unsigned int access_addr = 0;

		if(MAX_ACCESS_LIMIT < rest_size ){
			__i2c_read_reg_32(RAM_ADDR_REG_ADDR, &access_addr);
			__i2c_write_reg_32(RAM_ADDR_REG_ADDR, access_addr);
		}

		for(rest_size = array_size; 0 < rest_size && status == 0;){
			const int read_size = (rest_size < MAX_ACCESS_LIMIT ? rest_size : MAX_ACCESS_LIMIT );

			status = __i2c_read_reg_array(reg_addr, p, read_size );
			if(status)
				break;

			rest_size -= read_size;
			p += read_size;

			if(0 < rest_size ){
				access_addr += (read_size/4);
				__i2c_write_reg_32(RAM_ADDR_REG_ADDR, access_addr);
			}
		}
	}

	if(status){
		DEBUG_PRINT("frizz: i2c_read_reg_array() => %d\n", status );
	}
	return status;
}

static int __i2c_read_reg_uint_array(unsigned int reg_addr, unsigned int *array, int array_size) 
{
	int status = 0;
	unsigned char * const rx_buff = (unsigned char*)array;
	const int rx_size             = (array_size * 4);

	status = __frizz_i2c_read(reg_addr, rx_buff, rx_size);

	if (0 <= status) {
		const unsigned char* src         = rx_buff;
		unsigned int* dst                = array;
		const unsigned int* const beyond = dst + array_size;

		for(; dst < beyond; dst++, src+=4 ){
			*dst = CONVERT_UINT(src[0], src[1], src[2], src[3] );
		}
		status = 0;
	}
	return status;
}

int i2c_read_reg_uint_array(unsigned int reg_addr, unsigned int *array, int array_size) 
{
	int status = 0;

	{
		int rest_size = (array_size * 4);
		unsigned char* p = (unsigned char*)array;

		while(0 < rest_size && status == 0){
			const int read_size = (rest_size < MAX_ACCESS_LIMIT ? rest_size : MAX_ACCESS_LIMIT );

			status = __i2c_read_reg_uint_array(reg_addr, (unsigned int *)p, read_size/4 );
			if(status)
				break;
			rest_size -= read_size;
			p += read_size;
		}
	}

	if(status){
		DEBUG_PRINT("frizz: i2c_read_reg_uint_array() => %d\n", status );
	}
	return status;
}


int frizz_i2c_transfer_test(void)
{
	int ret;
	unsigned int tx_rx_data;

	keep_frizz_wakeup();
	ret = i2c_write_reg_32( RAM_ADDR_REG_ADDR, 0x00);
	if (ret == 0) {
		printf("frizz i2c write RAM_ADDR_REG_ADDR successful.\n");
	}
	else {
		printf("frizz i2c write RAM_ADDR_REG_ADDR failed.\n");
	}

	ret = i2c_read_reg_32(VER_REG_ADDR, &tx_rx_data);
	if (ret == 0){
		printf("frizz i2c read VER_REG_ADDR successful, receive data = %u.\n", tx_rx_data);
	}else {
		printf("frizz read VER_REG_ADDR failed.\n");
	}
	release_frizz_wakeup();

	return ret;
}

int frizz_i2c_probe(void)
{ 
	int ret = -1;
	keep_frizz_wakeup();
	ret = i2c_probe(I2C_FRIZZ_SLAVE_ADDRESS);
	release_frizz_wakeup();
	return ret;
} 

