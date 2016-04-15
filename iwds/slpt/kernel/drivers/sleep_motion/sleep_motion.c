#include <sleep_motion.h>

/* NOTE: 这不是一套实时，精准的测量算法 */
/* 算法流程 */
//1.lowpass:    数据进入一个LOWPASSDEEP Byte的平滑滤波
//2.variance:   以每（DEEP）个点数的集合做平方差 V。
//3.reset:      以V做段分域。分别取到 活跃状态/休息状态/睡眠状态 R。
//4.relocation: 取每STATE_DEEP个 R，做最大出现数取值，即段内粘性滤波 得到 Rl;
//5.recharge:   Rl状态仍有可能越阶跳变（POSITIVE <---> SLEEP），
//              对于越阶跳变的状态根据历史数据，做中和处理（置为REST）。
//              所以从活跃到睡眠，或者从睡眠到活跃，中间都有一段MAGIC的休息时间。
//              最后根据所处状态的时间长度，得到其余最终上报状态 久坐/深睡/脱下设备.
//6.report:     将这段时间（ODR*DEEP*STATE_DEEP）内的状态填入数组[0],数组后面为活跃状态程度（离散度variance）
//              返回DONE
//
//BUG级的策略： a)在进入DEVICE_OFF之前假设为非SLEEP状态，所以如果被判定为SLEEP状态，然后快速脱下手表，那么
//                便无法再检测到DEVICE_OFF,而是SLEEP_DEEP

static int limit = 0;
static int state_rest_counter = 0;
static int state_positive_counter = 0;
static int state_sleep_counter = 0;
static int buff[DEEP];
static int state_buff[STATE_DEEP];
static int lowpass[LOWPASSDEEP];
int state_report_buff[REPORT_STATE_DEEP];

static int state_report_history = STATE_POSITIVE;
    
static int device_off_counter = 0;
static int long_sit_counter = 0;
static int sleep_counter = 0;
static int deep_sleep_counter = 0;
static int state_report_history_counter = 0;
static int last_state = STATE_POSITIVE;
static int Belta_recharge = 0;

static int lowpass_index = 0;
static int last = 0;
static int counter = 0;
static int fair = 0;
static int sum = 0;
static int lowpass_input_count = 0;
static int lowpass_sum = 0;

static int lowpassfunc(int data);
static int relocation_machine(int nstate, int variance);
static int report_state_recharge (int nstate, int state_ratio, int* variance);
static void writeFile(char *filename, int data);

int sleep_motion(int data) {
    int ret = UNDONE;
    int countertmp = 0;
    int res = 0;

    writeFile("data", data);

    //1. fill lowpass buff
    if(lowpass_input_count < LOWPASSDEEP) {
        lowpassfunc(data);
        lowpass_input_count += 1;
        return UNDONE;
    }

    //2. fill variance buff
    res = lowpassfunc(data);
    buff[counter++] = res;
    sum += res;
    if(counter < DEEP) {
        //need more point for variance
        return UNDONE;
    }

    //3. calculate variance
    countertmp = counter;
    fair = sum / counter;
    sum = 0;

    while(counter > 0){
        sum += ((buff[counter - 1] - fair) * (buff[counter - 1] - fair));
        counter--;
    }
    last = sum / countertmp;
    res = last;

    writeFile("variance", last);

    //4. set state by variance rang
    if(last > POSITIVE_RANG) {
        last = STATE_POSITIVE;
    } else if(last >= REST_RANG) {
        last = STATE_REST;
    } else {last = STATE_SLEEP;}

    writeFile("reset", last);

    //5. relocation for kill some unsteady state.
    //6. report_func called by relocation, report_func will charge the final state for reporting.
    ret = relocation_machine((int)last, res);

    //clean all tmp values
    sum = 0;
    fair = 0;
    counter = 0;

    return ret;
}

static int lowpassfunc(int data) {
    int res = 0;
    lowpass_sum -= lowpass[lowpass_index];
    lowpass_sum += data;

    lowpass[lowpass_index] = data;
    lowpass_index += 1;

    if(lowpass_index == LOWPASSDEEP) {
        lowpass_index = 0;
    }
    res = lowpass_sum / LOWPASSDEEP;
    return res;
}

static int relocation_machine(int nstate, int variance) {
    int ret = -1;
    int i = 0;
    int state_counter = 0;
    //1.fill buff
    if(limit < STATE_DEEP)
        state_buff[limit] = variance;
    limit += 1;

    //2.pick out the max ratio state.
    //and set all the state by the max ratio state.
    switch(nstate) {
        case STATE_POSITIVE:
            state_positive_counter += 1;
            break;
        case STATE_REST:
            state_rest_counter += 1;
            break;
        case STATE_SLEEP:
            state_sleep_counter += 1;
    }

    if(limit == STATE_DEEP) {
        int state_max = -1;
        if(state_sleep_counter > state_rest_counter) {
            if(state_sleep_counter > state_positive_counter){
                state_max = STATE_SLEEP;
                state_counter = state_sleep_counter;
            }else {
                state_counter = state_positive_counter;
                state_max = STATE_POSITIVE;
            }
        }else {
            if(state_rest_counter > state_positive_counter) {
                state_max = STATE_REST;
                state_counter = state_rest_counter;
            }else {
                state_counter = state_positive_counter;
                state_max = STATE_POSITIVE;
            }
        }
        for(i = 0; i < STATE_DEEP; i++) {
            writeFile("relocation", state_max);
        }
        //3. state recharge and report it.
        ret = report_state_recharge(state_max, state_counter, state_buff);
        //clean all temp values
        state_sleep_counter = 0;
        state_rest_counter = 0;
        state_positive_counter = 0;
        limit = 0;

        return ret;
    }
    return UNDONE;
}
    
static int report_state_recharge (int nstate, int state_ratio, int* variance) {
    int state = nstate;
    int statetmp  = nstate;
    int i = 0;
    //1.copy the variance into the reoprt buff
    for(i = 0; i < STATE_DEEP; i++){
        state_report_buff[i + 1] = variance[i];
    }
	//Belta Recharge from 5.device_off
	if(Belta_recharge) {
		if(state_ratio == STATE_DEEP && nstate == STATE_SLEEP) {
			state = STATE_DEVICES_OFF;
			//when the device been wear again, must been POSITIVE/REST.
			//and rest or the counter/statetmp.
			last_state = STATE_REST;
			state_report_history_counter = 0;
			state_report_history = STATE_REST;
			goto finish;
		}else {
			Belta_recharge = 0;
		}
	}
    //2.Recharges state_history 
    if(state == last_state) {
        state_report_history_counter += 1;
    } else {
        state_report_history_counter = 0;
    }
    if(state_report_history_counter == HISTORY_LIMIT) {
        state_report_history_counter = 0;
        state_report_history = state;
    }
    last_state = state;
    //3.relocation the state by sticky
    if(state != state_report_history) {
        switch(state_report_history) {
            case STATE_REST:
                if(state == STATE_SLEEP) {
                    state = STATE_REST;
                }
                break;
            case STATE_POSITIVE:
                if(state == STATE_SLEEP) {
                    state = STATE_REST;
                }
                break;
            case STATE_SLEEP:
                if(state == STATE_POSITIVE) {
                    state = STATE_REST;
                }
                break;
            default:
                break;
        }
    }
    //4.must rest enough, then enter sleep mode, this is a magic.
    if(state == STATE_SLEEP) {
        sleep_counter += 1;
        if(sleep_counter < ENTER_SLEEP_LIMIT) {
        //not enough, I command u are resting. yeah, so magic.
            state = STATE_REST;
        } else {
        //now is under sleep state. charge deep sleep or not.
            if(state_ratio == STATE_DEEP) {
                deep_sleep_counter += 1;
            }else {
                deep_sleep_counter = 0;
            }
            if(deep_sleep_counter >= DEEP_SLEEP_LIMIT) {
                state = STATE_DEEP_SLEEP;
            }
        }
    }else {
        //if break the sleep counting, restart again, yeah, u need more rest.
        sleep_counter = 0;
    }

    //5.now, the rest state more than rest, also contain the device off state.
    //because of the state_sleep force to rest state, if do not rest enough
    if(state == STATE_REST) {
        long_sit_counter += 1;
        if(long_sit_counter >= LONG_SIT_LIMIT) {
            state = STATE_LONG_SIT;
        }
        //if long sit , and do not move hand forever, OK, you device is token off, whatever. 
        if(state_ratio == STATE_DEEP && statetmp == STATE_SLEEP) {
            device_off_counter += 1;
            if(device_off_counter >= DEVICE_OFF_LIMIT) {
                state = STATE_DEVICES_OFF;
				//if device_off, then enable Belta recharge.
				Belta_recharge = 1;
            }
        }
    }else {
        long_sit_counter = 0;
        device_off_counter = 0;
    }

finish:
    state_report_buff[0] = state;
    writeFile("report", state_report_buff[0]);
    return DONE;
}

static void writeFile(char *filename, int data) {
#ifdef DEBUG

#endif
    return;
}

