/* 
 * Morpheuz Sleep Monitor
 *
 * Copyright (c) 2013 James Fowler
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef MORPHEUZ_H_
#define MORPHEUZ_H_
	
#define VERSION "1.6"
	
#define FUDGE 4

#define POWER_NAP_MINUTES 27
#define POWER_NAP_SETTLE 2
#define POWER_NAP_SETTLE_THRESHOLD 1000

#define POWER_NAP_SETTLE_TIME "Power nap"
#define POWER_NAP_RUNNING     "Power nap: %d"
	
enum MorpKey {
	BIGGEST = 1,
    CTRL = 2,
    FROM = 3,
    TO = 4
};

enum CtrlValues {
	CTRL_ALARM = 1,
	CTRL_INVERSE = 2,
	CTRL_NORMAL = 4
};

#define SAMPLES_IN_TWO_MINUTES 48
#define ALARM_MAX 30
#define DISTRESS_WAIT_SEC 120
#define WINDOW_HEIGHT 168

void init_morpheuz(Window *window);
void deinit_morpheuz();
void do_alarm();
void self_monitor();
void reset_tick_service(bool second);
void set_smart_status_on_screen(bool show_special_text, char *special_text);
void invert_screen(bool inverse);
void power_nap_countdown();
void power_nap_check(uint16_t biggest);
void click_config_provider(Window *window);
void set_smart_status();
void fire_alarm();
void power_nap_reset();
void show_comms_state(bool connected);

#endif /* MORPHEUZ_H_ */
