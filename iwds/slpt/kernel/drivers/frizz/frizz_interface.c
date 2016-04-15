#include <common.h>
#include <asm/gpio.h>
#include <slpt.h>
#include <sleep_motion.h>
#include <slpt_app.h>
#include "frizz_i2c.h"
#include "frizz_packet.h"
#include "frizz_reg.h"
#include "libsensors_id.h"
static unsigned int fifo_data[MAX_HWFRIZZ_FIFO_SIZE];
static unsigned int timestamp;

unsigned long pedo_value = 0;
unsigned long gesture_value = 0;
unsigned long sleep_motion_enable = 0;

unsigned int frizz_wakeup_pin = 15;

//#define SLEEP_MOTION_TEST
#ifdef SLEEP_MOTION_TEST
int gdata_buff[500];
int counter;
#endif

void convert2float(unsigned int data, float *output) {
	float *fd = (float*) &data;
    *output = *fd;
}

void keep_frizz_wakeup(void) {
	if(frizz_wakeup_pin >= 0) {
		gpio_direction_output(frizz_wakeup_pin, 1);
		mdelay(10);
	}
}
void release_frizz_wakeup(void) {
	if(frizz_wakeup_pin >= 0) {
		gpio_direction_output(frizz_wakeup_pin, 0);
	}
}

static void analysis_packet(packet_t* packet)
{
	float data = 0;
	int fti = 0;
	switch(packet->header.sen_id) {
		case SENSOR_ID_ACCEL_PEDOMETER:
			printf("Accel Pedo :%d \n", packet->data[1]);
			pedo_value = packet->data[1];
			SLPT_SET_KEY(SLPT_K_FRIZZ_PEDO, packet->data[1]);
			break;
		case SENSOR_ID_PDR:
			//this sensor just for test
			printf("PDR Pedo :%d \n", packet->data[1]);
			//pedo_value = packet->data[1];
			//SLPT_SET_KEY(SLPT_K_FRIZZ_PEDO, packet->data[1]);
			break;
		case SENSOR_ID_GESTURE:
			printf("Gesture :%d \n", packet->data[1]);
			gesture_value = packet->data[1];
			SLPT_SET_KEY(SLPT_K_FRIZZ_GESTURE, packet->data[1]);
			slpt_mode_exit();
			break;
		case SENSOR_ID_ACCEL_RAW:
			if(sleep_motion_enable) {
				convert2float(packet->data[1], &data);
				fti = (int)(data * MAGNIFY);
#ifdef SLEEP_MOTION_TEST
				gdata_buff[counter++] = fti;
				if(counter == 500) {
					counter = 0;
					SLPT_SET_KEY(SLPT_K_SLEEP_MOTION, gdata_buff);
					slpt_mode_exit();
				}
#else
				if(sleep_motion(fti) == DONE) {
					SLPT_SET_KEY(SLPT_K_SLEEP_MOTION, state_report_buff);
					slpt_mode_exit();
				}
#endif
			}
			break;
		default:
			printf("Unhandle Sensor ID: %x \n", packet->header.sen_id);
			break;
	}
}
static void print_fifo_data(int num) {
#ifdef DEBUG
	int i;
	printf("dunm fifo data: ");

	for (i = 0; i < num; i++) {
		printf("%08x ", fifo_data[i]);
	}
	printf("\n");
#endif
	return;
}
static int analysis_fifo(unsigned int *fifo_data, int *index) {

    packet_t packet;
    int i, num;

    packet.header.w = fifo_data[*index];

    num = fifo_data[*index] & 0xFF;

    for (i = 0; i < num; i++) {
        packet.data[i] = fifo_data[i + *index + 1];
    }

    timestamp = packet.data[0];

    analysis_packet(&packet);

    return 0;
}

static int i2c_read_fifo_size(void) {

	int status;
	unsigned int fifo_cnr;
	status = i2c_read_reg_32(FIFO_CNR_REG_ADDR, &fifo_cnr);
	if (status == 0) {
		return (int) (fifo_cnr >> 16);
	} else {
		return -1;
	}
	return fifo_cnr;
}

int read_fifo(void) {

	int fifo_size;
	int i;

    //keep_frizz_wakeup();

	fifo_size = i2c_read_fifo_size();
	fifo_size = fifo_size > MAX_HWFRIZZ_FIFO_SIZE ? MAX_HWFRIZZ_FIFO_SIZE : fifo_size;

	if (fifo_size > 0) {
		//1.clean the buff
		for(i = 0; i < MAX_HWFRIZZ_FIFO_SIZE; i++) {
			fifo_data[i] = 0;
		}
		//2.read data
		i2c_read_reg_uint_array(FIFO_REG_ADDR, fifo_data, fifo_size);

		print_fifo_data(fifo_size);

		//3.analysis data
		for (i = 0; i < fifo_size; i++) {
			if (((fifo_data[i] >> 16) == 0xFF80) && ((fifo_data[i] & 0xff)
					<= (fifo_size - i))) {
				analysis_fifo(fifo_data, &i);
			}
		}

	}

	//release_frizz_wakeup();

	return 0;
}
