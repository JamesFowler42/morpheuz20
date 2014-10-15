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

#define VERSION 23
#define VERSION_TXT "2.3"

#define FUDGE 4

#define POWER_NAP_MINUTES 27
#define POWER_NAP_SETTLE 2
#define POWER_NAP_SETTLE_THRESHOLD 1000
#define SNOOZE_PERIOD_MS (9*60*1000)
#define FLASH_ALARM_MS 2000
#define WEEKEND_PERIOD (12*60*60)



#define BED_START GRect(-144-8, 15, 127, 70)
#define BED_FINISH GRect(8, 15, 127, 70)
#define SLEEPER_START GRect(25+144, 22, 110, 29)
#define SLEEPER_FINISH GRect(25, 22, 110, 29)
#define HEAD_START GRect(25, -22, 19, 16)
#define HEAD_FINISH GRect(25, 22, 19, 16)
#define TEXT_START GRect(26, 23, 92, 15)
#define TEXT_FINISH GRect(26, 70, 92, 15)
#define BLOCK_START GRect(0,91,144,78)
#define BLOCK_FINISH GRect(0,169,144,78)
#define MOON_FINISH GRect(6, 5, 58, 46)
#define MOON_START GRect(144+6, 72, 58, 46)

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

typedef enum {
  VIBE_LONG = 1,
  VIBE_DOUBLE = 2,
  VIBE_SHORT = 3,
  VIBE_SOS = 4
} VibeOptions;

#define DISTRESS_WAIT_SEC 10
#define WINDOW_HEIGHT 168

#define PERSIST_MEMORY_KEY 12121
#define PERSIST_CONFIG_KEY 12122
#define PERSIST_MEMORY_MS (5*60*1000)
#define PERSIST_CONFIG_MS 30000
#define SHORT_RETRY_MS 200
#define LONG_RETRY_MS 60000
#define NOTICE_DISPLAY_MS 7000
#define KEYBOARD_DISPLAY_MS 7000

#define LIMIT 54
#define DIVISOR 600

typedef struct {
	uint32_t base;
	uint16_t gone_off;
	uint8_t highest_entry;
	int8_t last_sent;
	uint16_t points[LIMIT];
	bool ignore[LIMIT];
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
void send_point(uint8_t point, uint16_t biggest, bool ignore);
void set_progress_based_on_persist();
InternalData *get_internal_data();
void read_internal_data();
void save_internal_data();
void show_record(bool recording);
void save_config_data(void *data);
void read_config_data();
ConfigData *get_config_data();
void show_notice(char *message);
bool cancel_alarm();
void fire_alarm();
bool snooze_alarm();
void init_alarm();
void show_keyboard();
void set_alarm_icon(bool show_icon);
void show_fatal(char *message);
void every_minute_processing(int min_no);
void trigger_config_save();
void do_vibes(VibeOptions opt);
void toggle_weekend_mode();
void notice_init();
void notice_deinit();
void set_text_layer_to_time(TextLayer *text_layer);
TextLayer* macro_text_layer_create(GRect frame, Layer *parent, GColor tcolor, GColor bcolor, GFont font, GTextAlignment text_alignment);
int32_t join_value(int16_t top, int16_t bottom);
int32_t dirty_checksum(void *data, uint8_t data_size);
void set_ignore_on_current_time_segment();
void show_ignore_state(bool ignore);
void resend_all_data();

#endif /* MORPHEUZ_H_ */
