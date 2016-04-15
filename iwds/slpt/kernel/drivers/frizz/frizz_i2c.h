#ifndef __FRIZZ_I2C_H__
#define __FRIZZ_I2C_H__

#include "frizz_reg.h"

#define CONVERT_UINT(imm0, imm1, imm2, imm3)				\
    (unsigned int)( (((imm0)&0xFF)<<24) | (((imm1)&0xFF)<<16) | (((imm2)&0xFF)<<8) | ((imm3)&0xFF) )

#define I2C_FRIZZ_SLAVE_ADDRESS   (0x1c)

#define I2C_BUFF_SIZE             (TRANS_BUFF_SIZE)   /*!< Max transfer buffer size (Byte)*/

int i2c_write_reg_32(unsigned int, unsigned int);

int i2c_read_reg_32(unsigned int, unsigned int*);

int i2c_write_reg_ram_data(unsigned int, unsigned int, unsigned char*, int);

int i2c_read_reg_array(unsigned int, unsigned char*, int);

int i2c_read_reg_uint_array(unsigned int, unsigned int*, int);

int frizz_i2c_transfer_test(void);

#endif
