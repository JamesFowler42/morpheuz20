/* 
 * Morpheuz Sleep Monitor
 *
 * Copyright (c) 2013-2015 James Fowler
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

#define VERSION 30
#define VERSION_TXT "3.0"
  
// Uncomment for release
#undef APP_LOG
#define APP_LOG(level, fmt, args...)

#define LAST_SENT_INIT -3

#define FUDGE 4

#define FROM_HR_DEF 6
#define FROM_MIN_DEF 30
#define TO_HR_DEF 7
#define TO_MIN_DEF 15

#define ALARM_PATTERN_MAX_REPEAT 5

#define POWER_NAP_MINUTES 27
#define POWER_NAP_SETTLE 2
#define POWER_NAP_SETTLE_THRESHOLD 1000
#define CLOCK_UPDATE_THRESHOLD 1000
#define SNOOZE_PERIOD_MS (9*60*1000)
#define MENU_ACTION_MS 500
#define WEEKEND_PERIOD (12*60*60)

#define BED_FINISH GRect(8, 17, 127, 70)
#define SLEEPER_FINISH GRect(25, 24, 110, 29)
#define HEAD_FINISH GRect(25, 24, 19, 16)
#define TEXT_FINISH GRect(26, 72, 92, 15)
#define BLOCK_FINISH GRect(0,169,144,78)
#define MOON_FINISH GRect(6, 5, 58, 46)
#define ICON_TOPS 1
#define ICON_PAD 5
#define ICON_PAD_BATTERY 4
#define ICON_BAR_WIDTH 118
#define BED_START GRect(-144-8, 17, 127, 70)
#define SLEEPER_START GRect(25+144, 24, 110, 29)
#define HEAD_START GRect(25, -24, 19, 16)
#define TEXT_START GRect(26, 25, 92, 15)
#define BLOCK_START GRect(0,91,144,78)
#define MOON_START GRect(144+6, 72, 58, 46)  

#ifdef PBL_COLOR 
  #define BACKGROUND_COLOR GColorDukeBlue  
  #define SETTING_BACKGROUND_COLOR BACKGROUND_COLOR
  #define ACTION_BAR_BACKGROUND_COLOR GColorWhite
  #define HIGHLIGHT_BG_COLOR GColorBlueMoon
  #define NON_HIGHLIGHT_BG_COLOR BACKGROUND_COLOR  
  #define HIGHLIGHT_FG_COLOR GColorWhite
  #define NON_HIGHLIGHT_FG_COLOR GColorWhite 
  #define FROM_TIME_COLOR GColorGreen
  #define TO_TIME_COLOR GColorRed
  #define START_TIME_COLOR GColorYellow
  #define PROGRESS_COLOR GColorLightGray
  #define ANALOGUE_COLOR GColorWhite
  #define FAILURE_COLOR GColorRed
#else
  #define BACKGROUND_COLOR GColorBlack  
  #define SETTING_BACKGROUND_COLOR GColorWhite
  #define ACTION_BAR_BACKGROUND_COLOR GColorBlack
  #define HIGHLIGHT_BG_COLOR GColorBlack
  #define NON_HIGHLIGHT_BG_COLOR GColorWhite  
  #define HIGHLIGHT_FG_COLOR GColorWhite
  #define NON_HIGHLIGHT_FG_COLOR GColorBlack  
  #define FROM_TIME_COLOR GColorWhite
  #define TO_TIME_COLOR GColorWhite
  #define START_TIME_COLOR GColorWhite
  #define PROGRESS_COLOR GColorWhite
  #define ANALOGUE_COLOR GColorWhite
  #define FAILURE_COLOR GColorWhite
#endif

// These save space and time to run and a direct cast is claimed to be supported in the documentation
#define bitmap_layer_get_layer_jf(x) ((Layer *)(x))
#define text_layer_get_layer_jf(x) ((Layer *)(x))
#define inverter_layer_get_layer_jf(x) ((Layer *)(x))
#define menu_layer_get_layer_jf(x) ((Layer *)(x))

#define to_mins(h,m) (((h) * 60) + (m))

enum MorpKey {
  KEY_POINT = 1,
  KEY_CTRL = 2,
  KEY_FROM = 3,
  KEY_TO = 4,
  KEY_BASE = 5,
  KEY_VERSION = 6,
  KEY_GONEOFF = 7,
  KEY_TRANSMIT = 8
};

enum CtrlValues {
  CTRL_TRANSMIT_DONE = 1,
  CTRL_VERSION_DONE = 2,
  CTRL_GONEOFF_DONE = 4,
  CTRL_DO_NEXT = 8,
  CTRL_SET_LAST_SENT = 16
};

typedef enum {
  IS_COMMS = 0,
  IS_RECORD,
  IS_IGNORE,
  IS_ACTIVITY,
  IS_WEEKEND,
  IS_ALARM,
  IS_ALARM_RING,
  IS_BLUETOOTH,
  IS_EXPORT
} IconState;

#define MAX_ICON_STATE 9

#define PERSIST_MEMORY_KEY 12121
#define PERSIST_CONFIG_KEY 12122
#define PERSIST_MEMORY_MS (5*60*1000)
#define PERSIST_CONFIG_MS 30000
#define SHORT_RETRY_MS 200
#define LONG_RETRY_MS 60000
#define NOTICE_DISPLAY_MS 7000
#define FIVE_MINUTES_MS (5*60*1000)

#define LIMIT 54
#define DIVISOR 600

#define LOGO_BED_ANIMATION 0
#define LOGO_SLEEPER_ANIMATION 1
#define LOGO_HEAD_ANIMATION 2
#define LOGO_TEXT_ANIMATION 3
#define BLOCK_ANIMATION 4
#define MAX_ANIMATIONS 5

#define BUFFER_SIZE 40

#define TWENTY_FOUR_HOURS_IN_SECONDS (24*60*60)
#define TEN_HOURS_IN_SECONDS (10*60*60)
#define WAKEUP_AUTO_RESTART 1
#define WAKEUP_FOR_TRANSMIT 2
#define ONE_MINUTE 60

typedef struct {
  uint32_t base;
  uint16_t gone_off;
  uint8_t highest_entry;
  int8_t last_sent;
  uint16_t points[LIMIT];
  bool ignore[LIMIT];
  bool has_been_reset;
  bool gone_off_sent;
  bool transmit_sent;
  int32_t tm_gmtoff;
} InternalData;

typedef struct {
  bool invert;
  bool analogue;
  bool smart;
  bool auto_reset;
  uint8_t autohr;
  uint8_t automin;
  uint8_t fromhr;
  uint8_t frommin;
  uint8_t tohr;
  uint8_t tomin;
  uint32_t from;
  uint32_t to;
  time_t weekend_until;
} ConfigData;

typedef struct {
  BitmapLayer *layer;
  GBitmap *bitmap;
} BitmapLayerComp;

void init_morpheuz(Window *window);
void set_smart_status_on_screen(bool show_special_text, char *special_text);
void invert_screen();
void revive_clock_on_movement(uint16_t last_movement);
void power_nap_countdown();
void power_nap_check(uint16_t biggest);
void click_config_provider(Window *window);
void set_smart_status();
void power_nap_reset();
void reset_sleep_period();
void server_processing(uint16_t biggest);
void set_progress();
InternalData *get_internal_data();
void read_internal_data();
void save_internal_data();
void save_config_data(void *data);
void read_config_data();
ConfigData *get_config_data();
void show_notice(uint32_t resource_id);
void hide_notice_layer(void *data);
void cancel_alarm();
void fire_alarm();
void snooze_alarm();
void init_alarm();
uint16_t every_minute_processing();
void toggle_weekend_mode();
TextLayer* macro_text_layer_create(GRect frame, Layer *parent, GColor tcolor, GColor bcolor, GFont font, GTextAlignment text_alignment);
int32_t join_value(int16_t top, int16_t bottom);
int32_t dirty_checksum(void *data, uint8_t data_size);
void set_ignore_on_current_time_segment();
void resend_all_data(bool silent);
void bed_visible(bool value);
bool is_animation_complete();
void show_menu();
void hide_menu();
void toggle_power_nap();
void close_morpheuz();
bool is_notice_showing();
void macro_bitmap_layer_create(BitmapLayerComp *comp, GRect frame, Layer *parent, uint32_t resource_id, bool visible);
void macro_bitmap_layer_destroy(BitmapLayerComp *comp);
void wakeup_init();
void wakeup_toggle();
void set_next_wakeup();
uint8_t twenty_four_to_twelve(uint8_t hour);
void post_init_hook();
void show_set_alarm();
void trigger_config_save();
bool is_doing_powernap();
void open_comms();
void start_worker();
void set_icon(bool enabled, IconState icon);
bool get_icon(IconState icon);
void set_failure_text(char *failure);

#ifdef PBL_COLOR 
  time_t time_local(time_t *timein);
#endif

#endif /* MORPHEUZ_H_ */
