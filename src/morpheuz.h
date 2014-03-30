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

#define VERSION 21
#define VERSION_TXT "2.1"

#define FUDGE 4

#define POWER_NAP_MINUTES 27
#define POWER_NAP_SETTLE 2
#define POWER_NAP_SETTLE_THRESHOLD 1000
#define SNOOZE_PERIOD_MS (9*60*1000)
#define FLASH_ALARM_MS 2000
#define WEEKEND_PERIOD (12*60*60)

#define POWER_NAP_SETTLE_TIME "Power nap"
#define POWER_NAP_RUNNING     "Power nap: %d"
#define WEEKEND_TEXT 		  "Weekend"
#define NOTICE_TIMER_RESET_ALARM "Sleep well!\nChart reset\nAlarm set"
#define NOTICE_TIMER_RESET_NOALARM "Sleep well!\nChart reset\nNO ALARM"
#define NOTICE_STARTED_POWER_NAP "\nPower nap\nstarted"
#define NOTICE_STOPPED_POWER_NAP "\nPower nap\nstopped"
#define NOTICE_TIME_TO_WAKE_UP "\nTime to\nwake up!"
#define NOTICE_WELCOME "Morpheuz\nSleep Monitor\nVersion %s"
#define NOTICE_ALARM_CANCELLED "\nAlarm\nCancelled"
#define NOTICE_END_OF_RECORDING "End of recording\nReset to start again"
#define NOTICE_RESET_TO_START_USING "Reset to start\nrecording"
#define NOTICE_SNOOZE_ACTIVATED "\nSnooze\n9 minutes"
#define NOTICE_STARTED_WEEKEND "Weekend mode\nNo alarm for 12hrs"
#define NOTICE_STOPPED_WEEKEND "Weekend mode cancelled"
#define NOTICE_NEED_SMART_ALARM "No Smart alarm: no weekend mode!"

#define FATAL_ACCEL_CRASH "Morpheuz has found an error: The accelerometer is not returning any data. Please contact support from the Pebble app on your phone. Shutdown and restart your Pebble.\nHold back to leave Morpheuz."

enum MorpKey {
	KEY_POINT = 1,
	KEY_CTRL = 2,
	KEY_FROM = 3,
	KEY_TO = 4,
	KEY_BASE = 5,
	KEY_VERSION = 6,
	KEY_GONEOFF = 7
};

enum CtrlValues {
	CTRL_RESET = 1,
	CTRL_INVERSE = 2,
	CTRL_NORMAL = 4
};

#define DISTRESS_WAIT_SEC 10
#define WINDOW_HEIGHT 168

#define PERSIST_MEMORY_KEY 12121
#define PERSIST_CONFIG_KEY 12122
#define PERSIST_MEMORY_MS (5*60*1000)
#define PERSIST_CONFIG_MS 30000
#define SHORT_RETRY_MS 200
#define LONG_RETRY_MS 60000
#define NOTICE_DISPLAY_MS 7000
#define NOTICE_DISPLAY_SHORT_MS 2500
#define KEYBOARD_DISPLAY_MS 7000

#define LIMIT 54
#define DIVISOR 600

typedef struct {
	uint32_t base;
	uint16_t gone_off;
	uint8_t highest_entry;
	int8_t last_sent;
	uint16_t points[LIMIT];
	bool has_been_reset;
	bool gone_off_sent;
} InternalData;

typedef struct {
	bool invert;
	bool smart;
	uint8_t fromhr;
	uint8_t frommin;
	uint8_t tohr;
	uint8_t tomin;
	uint32_t from;
	uint32_t to;
	time_t weekend_until;
} ConfigData;

void init_morpheuz(Window *window);
void deinit_morpheuz();
void self_monitor();
void set_smart_status_on_screen(bool show_special_text, char *special_text);
void invert_screen();
void power_nap_countdown();
void power_nap_check(uint16_t biggest);
void click_config_provider(Window *window);
void set_smart_status();
void power_nap_reset();
void show_comms_state(bool connected);
void set_config_data(int32_t iface_from, int32_t iface_to, bool iface_invert);
void reset_sleep_period();
void server_processing(uint16_t biggest);
void transmit_next_data(void *data);
void set_progress(uint8_t progress_percent);
void send_base(uint32_t base);
void send_goneoff();
void send_version(void *data);
void send_point(uint8_t point, uint16_t biggest);
void set_progress_based_on_persist();
InternalData *get_internal_data();
void read_internal_data();
void save_internal_data();
void show_record(bool recording);
void save_config_data(void *data);
void read_config_data();
ConfigData *get_config_data();
void show_notice(char *message, bool short_time);
bool cancel_alarm();
void fire_alarm();
bool snooze_alarm();
void init_alarm();
void show_keyboard();
void set_alarm_icon(bool show_icon);
void show_fatal(char *message);
void every_minute_processing(int min_no);
void trigger_config_save();
void vibes_sos();
void toggle_weekend_mode();

#endif /* MORPHEUZ_H_ */
