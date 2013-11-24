/*
 * morpheuz.h
 *
 *  Created on: 14 Nov 2013
 *      Author: james
 */

#ifndef MORPHEUZ_H_
#define MORPHEUZ_H_
	
#define BUFF_SIZE 28
	
enum MorpKey {
	BIGGEST = 1,
    ALARM = 2
};

void init_morpheuz();
void deinit_morpheuz();
void do_alarm();
void reset_tick_service(bool second);

#endif /* MORPHEUZ_H_ */
