#ifndef __SLEEP_MOTION_H__
#define __SLEEP_MOTION_H__

#define MAGNIFY 100

#define STATE_DEEP_SLEEP  6
#define STATE_LONG_SIT    5
#define STATE_DEVICES_OFF 4
#define STATE_POSITIVE    3
#define STATE_REST        2
#define STATE_SLEEP       1

#define LOWPASSDEEP       10    //lowpass buff deep
#define DEEP              500   //variance deep
#define STATE_DEEP        7
#define REPORT_STATE_DEEP (STATE_DEEP + 1)

#define POSITIVE_RANG     5000  //larger than POSITIVE_RANGE is POSITIVE STATE  0.5 * magnify * magnify
#define REST_RANG         100   //larger than  REST_RANG is REST STATE           0.01 * magnify * magnify
#define SLEEP_RANG        100   //less than SLEEP_RANG is SLEEP STATE

#define SENSOR_RATE       100   //Ms---unused

#define DEEP_SLEEP_LIMIT  2
#define DEVICE_OFF_LIMIT  2     // 15 mins
#define LONG_SIT_LIMIT    15    // Minus
#define HISTORY_LIMIT     3
#define ENTER_SLEEP_LIMIT 3

#define DONE              0
#define UNDONE            -1

extern int state_report_buff[STATE_DEEP + 1];
int sleep_motion(int data);

//#define DEBUG

#endif

