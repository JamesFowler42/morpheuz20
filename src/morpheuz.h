/*
 * morpheuz.h
 *
 *  Created on: 14 Nov 2013
 *      Author: james
 */

#ifndef MORPHEUZ_H_
#define MORPHEUZ_H_
	
#define VERSION "v0.8"
	
#define BUFF_SIZE 28
	
enum MorpKey {
	BIGGEST = 1,
    ALARM = 2
};

#define SAMPLES_IN_ONE_MINUTE 24
#define ALARM_MAX 30
#define DISTRESS_WAIT_SEC 120
#define WINDOW_HEIGHT 168

void init_morpheuz();
void deinit_morpheuz();
void do_alarm();
void reset_tick_service(bool second);
void set_alert_code(int new_alert_code);

#endif /* MORPHEUZ_H_ */
