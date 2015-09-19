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

#include "pebble.h"

#define VERSION 35
#define VERSION_TXT "3.5"

// Comment out for production build - leaves errors on BASALT and nothing on APLITE as this is much tighter for memory
//#define TESTING_BUILD

#ifdef TESTING_BUILD
  #define LOG_ERROR(fmt, args...) app_log(APP_LOG_LEVEL_ERROR, "", 0, fmt, ## args)
  #define LOG_WARN(fmt, args...) app_log(APP_LOG_LEVEL_WARNING, "", 0, fmt, ## args)
  #define LOG_INFO(fmt, args...) app_log(APP_LOG_LEVEL_INFO, "", 0, fmt, ## args)
  #define LOG_DEBUG(fmt, args...) app_log(APP_LOG_LEVEL_DEBUG, "", 0, fmt, ## args)
#else
  #ifdef PBL_PLATFORM_BASALT 
    #define LOG_ERROR(fmt, args...) app_log(APP_LOG_LEVEL_ERROR, "", 0, fmt, ## args)
  #else
    #define LOG_ERROR(fmt, args...)
  #endif
  #define LOG_WARN(fmt, args...) 
  #define LOG_INFO(fmt, args...) 
  #define LOG_DEBUG(fmt, args...) 
#endif
  
// Read clock mode from OS
//#define IS_24_HOUR_MODE clock_is_24h_style()
#define IS_24_HOUR_MODE false

// Only do this to make greping for external functions easier (lot of space to be saved with statics)
#define EXTFN

#define LAST_SENT_INIT -4

#define FUDGE 4

#define FROM_HR_DEF 6
#define FROM_MIN_DEF 30
#define TO_HR_DEF 7
#define TO_MIN_DEF 15

#define ALARM_PATTERN_MAX_REPEAT 5

#define TIMELINE_LAUNCH_USE 0
#define TIMELINE_LAUNCH_SLEEP_NOW 1
#define TIMELINE_LAUNCH_CLEAR_AUTOSLEEP 2

#define DUMMY_PREVIOUS_TO_PHONE 0xFFFFFFFF

#define POWER_NAP_MINUTES 27
// #define POWER_NAP_MINUTES 1 
  
#define POWER_NAP_SETTLE 2
#define POWER_NAP_SETTLE_THRESHOLD 1000
#define CLOCK_UPDATE_THRESHOLD 1000
#define SNOOZE_PERIOD_MS (9*60*1000)
#define POST_MENU_ACTION_DISPLAY_UPDATE_MS 900
#define MENU_ACTION_MS 750
#define MENU_ACTION_HIDE_MS 500
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
  #define HIGHLIGHT_BG_COLOR GColorBlack
  #define NON_HIGHLIGHT_BG_COLOR GColorBlue
  #define HIGHLIGHT_FG_COLOR GColorWhite
  #define NON_HIGHLIGHT_FG_COLOR GColorWhite
  #define FROM_TIME_COLOR GColorGreen
  #define TO_TIME_COLOR GColorRed
  #define START_TIME_COLOR GColorYellow
  #define PROGRESS_COLOR GColorWhite
  #define ANALOGUE_COLOR GColorWhite
  #define FAILURE_COLOR GColorRed
  #define BATTERY_BAR_COLOR GColorYellow
  #define BATTERY_BAR_COLOR_CRITICAL GColorRed
  #define BATTERY_BAR_COLOR_WARN GColorRajah
  #define BAR_CHART_MARKS GColorLightGray
  #define ANIMATE_MAIN_DURATION 500
  #define ANIMATE_HEAD_DURATION 250
  #define ANIMATE_ANALOGUE_DURATION 375
  #define INTER_TEXT_COLOR_MS 187
  #define MENU_TEXT_COLOR GColorWhite
  #define MENU_HIGHLIGHT_BACKGROUND_COLOR GColorBlack
  #define MENU_BACKGROUND_COLOR BACKGROUND_COLOR
  #define MENU_HEAD_COLOR GColorWhite
  #define MINUTE_HAND_COLOR GColorWhite
  #define HOUR_HAND_COLOR GColorPictonBlue
  #define MINUTE_HAND_OUTLINE GColorWhite
  #define HOUR_HAND_OUTLINE GColorPictonBlue
  #define CENTRE_OUTLINE GColorWhite
  #define CENTRE_COLOR GColorPictonBlue
  #define MINUTE_MARK_COLOR GColorDarkGray
  #define HOUR_MARK_COLOR GColorWhite
  #define MINUTE_STEP 24
  #define PROGRESS_STEP 24
  #define COPYRIGHT_COLOR BACKGROUND_COLOR
#else
  #define BACKGROUND_COLOR GColorBlack
  #define SETTING_BACKGROUND_COLOR GColorBlack
  #define ACTION_BAR_BACKGROUND_COLOR GColorBlack
  #define HIGHLIGHT_BG_COLOR GColorWhite
  #define NON_HIGHLIGHT_BG_COLOR GColorBlack
  #define HIGHLIGHT_FG_COLOR GColorBlack
  #define NON_HIGHLIGHT_FG_COLOR GColorWhite
  #define FROM_TIME_COLOR GColorWhite
  #define TO_TIME_COLOR GColorWhite
  #define START_TIME_COLOR GColorWhite
  #define PROGRESS_COLOR GColorWhite
  #define ANALOGUE_COLOR GColorWhite
  #define FAILURE_COLOR GColorWhite
  #define BATTERY_BAR_COLOR GColorWhite
  #define BATTERY_BAR_COLOR_CRITICAL GColorWhite
  #define BATTERY_BAR_COLOR_WARN GColorWhite
  #define BAR_CHART_MARKS GColorWhite
  #define ANIMATE_MAIN_DURATION 1000
  #define ANIMATE_HEAD_DURATION 500
  #define ANIMATE_ANALOGUE_DURATION 750
  #define PRE_ANIMATE_DELAY 1683
  #define MENU_HEAD_COLOR GColorBlack
  #define MINUTE_HAND_COLOR GColorWhite
  #define HOUR_HAND_COLOR GColorWhite
  #define MINUTE_HAND_OUTLINE GColorBlack
  #define HOUR_HAND_OUTLINE GColorBlack
  #define CENTRE_OUTLINE GColorBlack
  #define CENTRE_COLOR GColorWhite
  #define MINUTE_MARK_COLOR GColorWhite
  #define HOUR_MARK_COLOR GColorWhite
  #define MINUTE_STEP 24
  #define PROGRESS_STEP 12
  #define COPYRIGHT_COLOR GColorWhite
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
  KEY_TRANSMIT = 8,
  KEY_AUTO_RESET = 9
};

enum CtrlValues {
  CTRL_TRANSMIT_DONE = 1,
  CTRL_VERSION_DONE = 2,
  CTRL_GONEOFF_DONE = 4,
  CTRL_DO_NEXT = 8,
  CTRL_SET_LAST_SENT = 16,
  CTRL_LAZARUS = 32
};

typedef enum {
  IS_COMMS = 0,
  IS_RECORD,
  IS_IGNORE,
  IS_WEEKEND,
  IS_ALARM,
  IS_ALARM_RING,
  IS_BLUETOOTH,
  IS_EXPORT
} IconState;

#define MAX_ICON_STATE 8

#define PERSIST_MEMORY_KEY 12121
#define PERSIST_CONFIG_KEY 12122
#define PERSIST_PRESET_KEY 12123
#define PERSIST_MEMORY_MS (5*60*1000)
#define PERSIST_CONFIG_MS 30000
#define SHORT_RETRY_MS 200
#define LONG_RETRY_MS 60000
#define NOTICE_DISPLAY_MS 7000
#define FIVE_MINUTES_MS (5*60*1000)
#define TEN_SECONDS_MS (10*1000)
#define COMPLETE_OUTSTANDING_MS (15*1000)
#define FIVE_MINUTES (5*60)
#define FIVE_SECONDS 5
#define VERSION_SEND_INTERVAL_MS (1000)
#define VERSION_SEND_SLOW_INTERVAL_MS (60*1000)

#define LIMIT 60
#define DIVISOR 600

#define LOGO_BED_ANIMATION 0
#define LOGO_SLEEPER_ANIMATION 1
#define LOGO_HEAD_ANIMATION 2
#define LOGO_TEXT_ANIMATION 3
#define BLOCK_ANIMATION 4
#define MAX_ANIMATIONS 5

#define BUFFER_SIZE 50
  
#define TWENTY_FOUR_HOURS_IN_SECONDS (24*60*60)
#define ELEVEN_HOURS_IN_SECONDS (11*60*60)
#define WAKEUP_AUTO_RESTART 1
#define WAKEUP_FOR_TRANSMIT 2
#define WAKEUP_LAZARUS 3
#define ONE_MINUTE 60

// Change INTERNAL_VER only if the InternalData struct changes
#define INTERNAL_VER 42
typedef struct {
  uint8_t internal_ver;
  uint32_t base;
  uint16_t gone_off;
  uint8_t highest_entry;
  int8_t last_sent;
  uint16_t points[LIMIT];
  bool ignore[LIMIT];
  bool has_been_reset;
  bool gone_off_sent;
  bool transmit_sent;
} InternalData;

// Change the CONFIG_VER only if the ConfigData struct changes
#define CONFIG_VER 42
typedef struct {
  uint8_t config_ver;
  bool invert;
  bool analogue;
  bool smart;
  bool auto_reset;
  bool lazarus;
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

// Externals
ConfigData *get_config_data();
InternalData *get_internal_data();
TextLayer* macro_text_layer_create(GRect frame, Layer *parent, GColor tcolor, GColor bcolor, GFont font, GTextAlignment text_alignment);
bool get_icon(IconState icon);
bool is_animation_complete();
bool is_doing_powernap();
bool is_monitoring_sleep();
bool is_notice_showing();
int main(void);
int32_t dirty_checksum(void *data, uint8_t data_size);
int32_t join_value(int16_t top, int16_t bottom);
uint16_t every_minute_processing();
uint8_t twenty_four_to_twelve(uint8_t hour);
void analogue_minute_tick();
void analogue_powernap_text(char *text);
void analogue_set_base(time_t base);
void analogue_set_progress(uint8_t progress_level_in);
void analogue_set_smart_times();
void analogue_visible(bool visible, bool call_post_init);
void analogue_window_load(Window *window);
void analogue_window_unload();
void bed_visible(bool value);
void cancel_alarm();
void click_config_provider(Window *window);
void close_morpheuz();
void copy_time_range_into_field(char *field, size_t fsize, uint8_t fromhr, uint8_t frommin, uint8_t tohr, uint8_t tomin);
void fire_alarm();
void hide_notice_layer(void *data);
void init_morpheuz();
void invert_screen();
void lazarus();
void macro_bitmap_layer_create(BitmapLayerComp *comp, GRect frame, Layer *parent, uint32_t resource_id, bool visible);
void macro_bitmap_layer_destroy(BitmapLayerComp *comp);
void manual_shutdown_request();
void open_comms();
void post_init_hook(void *data);
void power_nap_check(uint16_t biggest);
void power_nap_countdown();
void power_nap_reset();
void read_config_data();
void read_internal_data();
void resend_all_data(bool invoked_by_change_of_time);
void reset_sleep_period();
void revive_clock_on_movement(uint16_t last_movement);
void save_config_data(void *data);
void save_internal_data();
void server_processing(uint16_t biggest);
void set_icon(bool enabled, IconState icon);
void set_ignore_on_current_time_segment();
void set_next_wakeup();
void set_progress();
void set_smart_status();
void set_smart_status_on_screen(bool smart_alarm_on, char *special_text);
void show_alarm_buttons(bool value);
void show_menu();
void show_notice(uint32_t resource_id);
void show_preset_menu();
void show_set_alarm();
void snooze_alarm();
void toggle_power_nap();
void toggle_weekend_mode();
void trigger_config_save();
void wakeup_init();
void wakeup_toggle();

#endif /* MORPHEUZ_H_ */
