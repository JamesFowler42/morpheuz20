/* 
 * Morpheuz Sleep Monitor
 *
 * Copyright (c) 2013-2016 James Fowler
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
#include "pebble_process_info.h"

// Comment out for production build - leaves errors on BASALT/CHALK and nothing on APLITE as this is much tighter for memory
//#define TESTING_BUILD

// Comment out for production build - shows heap where date or smart alarm times show
//#define TESTING_MEMORY_LEAK

#ifdef TESTING_BUILD
  #define LOG_ERROR(fmt, args...) app_log(APP_LOG_LEVEL_ERROR, "", 0, fmt, ## args)
  #define LOG_WARN(fmt, args...) app_log(APP_LOG_LEVEL_WARNING, "", 0, fmt, ## args)
  #define LOG_INFO(fmt, args...) app_log(APP_LOG_LEVEL_INFO, "", 0, fmt, ## args)
  #define LOG_DEBUG(fmt, args...) app_log(APP_LOG_LEVEL_DEBUG, "", 0, fmt, ## args)
#else
  #ifndef PBL_PLATFORM_APLITE 
    #define LOG_ERROR(fmt, args...) app_log(APP_LOG_LEVEL_ERROR, "", 0, fmt, ## args)
  #else
    #define LOG_ERROR(fmt, args...)
  #endif
  #define LOG_WARN(fmt, args...) 
  #define LOG_INFO(fmt, args...) 
  #define LOG_DEBUG(fmt, args...) 
#endif

#ifdef PBL_MICROPHONE
  #define VOICE_SUPPORTED
#else 
  #define is_voice_system_active() (false)
#endif

// APLITE is optimised for space, BASALT/CHALK and above are optimised for battery life
#ifndef PBL_PLATFORM_APLITE
  #define CACHE_ICONS
  #define ENABLE_CHART_VIEWER
#endif
  
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
#define TIMELINE_LAUNCH_CHART 3

#define DUMMY_PREVIOUS_TO_PHONE 0xFFFFFFFF

#define POWER_NAP_MINUTES 27
// #define POWER_NAP_MINUTES 1 
  
#define POWER_NAP_SETTLE 2
#define CLOCK_UPDATE_THRESHOLD AWAKE_ABOVE
#define SNOOZE_PERIOD_MS (9*60*1000)
#define POST_MENU_ACTION_DISPLAY_UPDATE_MS 900
#define MENU_ACTION_MS 750
#define MENU_ACTION_HIDE_MS 500

#define ICON_PAD 5
#define ICON_PAD_BATTERY 4
  
#define PRE_ANIMATE_DELAY 1683
  
#define ICON_TOPS                    PBL_IF_RECT_ELSE(1, 26) 
#define ICON_BAR_WIDTH               PBL_IF_RECT_ELSE(118,88)

// Colours
#define BACKGROUND_COLOR             PBL_IF_COLOR_ELSE(GColorDukeBlue, GColorBlack)
#define SETTING_BACKGROUND_COLOR     BACKGROUND_COLOR
#define ACTION_BAR_BACKGROUND_COLOR  PBL_IF_COLOR_ELSE(GColorWhite, GColorBlack)
#define HIGHLIGHT_BG_COLOR           PBL_IF_COLOR_ELSE(GColorBlack, GColorWhite)
#define NON_HIGHLIGHT_BG_COLOR       PBL_IF_COLOR_ELSE(GColorBlue, GColorBlack)
#define HIGHLIGHT_FG_COLOR           PBL_IF_COLOR_ELSE(GColorWhite, GColorBlack)
#define NON_HIGHLIGHT_FG_COLOR       PBL_IF_COLOR_ELSE(GColorWhite, GColorWhite)
#define FROM_TIME_COLOR              PBL_IF_COLOR_ELSE(GColorGreen, GColorWhite)
#define TO_TIME_COLOR                PBL_IF_COLOR_ELSE(GColorRed, GColorWhite)
#define START_TIME_COLOR             PBL_IF_COLOR_ELSE(GColorYellow, GColorWhite)
#define PROGRESS_COLOR               PBL_IF_COLOR_ELSE(GColorWhite, GColorWhite)
#define ANALOGUE_COLOR               PBL_IF_COLOR_ELSE(GColorWhite, GColorWhite)
#define BATTERY_BAR_COLOR            PBL_IF_COLOR_ELSE(GColorYellow, GColorWhite)
#define BAR_CHART_MARKS              PBL_IF_COLOR_ELSE(GColorLightGray, GColorWhite)
#define COPYRIGHT_COLOR              PBL_IF_COLOR_ELSE(BACKGROUND_COLOR, GColorWhite)
#define MENU_HEAD_COLOR              PBL_IF_COLOR_ELSE(GColorWhite,GColorBlack)
#define MINUTE_HAND_COLOR            PBL_IF_COLOR_ELSE(GColorWhite,GColorWhite)
#define HOUR_HAND_COLOR              PBL_IF_COLOR_ELSE(GColorPictonBlue,GColorWhite)
#define MINUTE_HAND_OUTLINE          PBL_IF_COLOR_ELSE(GColorWhite,GColorBlack)
#define HOUR_HAND_OUTLINE            PBL_IF_COLOR_ELSE(GColorPictonBlue,GColorBlack)
#define CENTRE_OUTLINE               PBL_IF_COLOR_ELSE(GColorWhite,GColorBlack)
#define CENTRE_COLOR                 PBL_IF_COLOR_ELSE(GColorPictonBlue,GColorWhite)
#define MINUTE_MARK_COLOR            PBL_IF_COLOR_ELSE(GColorDarkGray,GColorWhite)
#define HOUR_MARK_COLOR              PBL_IF_COLOR_ELSE(GColorWhite,GColorWhite)
#define ERROR_COLOR                  PBL_IF_COLOR_ELSE(GColorRed,GColorWhite)

// Colour only colours (it's not my fault I only wrote the comment)
#ifdef PBL_COLOR 
  #define BATTERY_BAR_COLOR_CRITICAL GColorRed
  #define BATTERY_BAR_COLOR_WARN GColorRajah
  #define MENU_TEXT_COLOR GColorWhite
  #define MENU_HIGHLIGHT_BACKGROUND_COLOR GColorBlack
  #define MENU_BACKGROUND_COLOR BACKGROUND_COLOR
  #define CHART_BACKGROUND_COLOR GColorDarkGray
  #define CHART_INDICATOR_BACKGROUND_COLOR GColorDarkGray
  #define CHART_AWAKE_COLOR GColorPictonBlue
  #define CHART_LIGHT_COLOR GColorBlueMoon
  #define CHART_DEEP_COLOR GColorDukeBlue
  #define CHART_IGNORE_COLOR GColorLightGray
  #define CHART_SLEEP_AWAKE_MARKER_COLOR GColorIcterine
  #define CHART_TRIM_COLOR GColorWhite
  #define CHART_SLEEP_SMART_EARLIEST_COLOR GColorGreen
  #define CHART_SLEEP_SMART_LATEST_COLOR GColorRed
#endif

// Animates
#define ANIMATE_MAIN_DURATION        PBL_IF_COLOR_ELSE(500, 1000)
#define ANIMATE_HEAD_DURATION        PBL_IF_COLOR_ELSE(250, 500)
#define ANIMATE_ANALOGUE_DURATION    PBL_IF_COLOR_ELSE(375, 750)

// Colour only animates
#define INTER_TEXT_COLOR_MS 187

// General
#define MINUTE_STEP 24
#define PROGRESS_STEP                PBL_IF_COLOR_ELSE(24,12)

// These save space and time to run and a direct cast is claimed to be supported in the documentation
#define bitmap_layer_get_layer_jf(x) ((Layer *)(x))
#define text_layer_get_layer_jf(x) ((Layer *)(x))
#define inverter_layer_get_layer_jf(x) ((Layer *)(x))
#define menu_layer_get_layer_jf(x) ((Layer *)(x))

#define to_mins(h,m) (((h) * 60) + (m))

#define tolower(a) ((('A' <= a) && (a <= 'Z')) ? ('a' + (a - 'A')) : (a))

#define  KEY_POINT  MESSAGE_KEY_keyPoint
#define  KEY_CTRL  MESSAGE_KEY_keyCtrl
#define  KEY_FROM  MESSAGE_KEY_keyFrom
#define  KEY_TO MESSAGE_KEY_keyTo
#define  KEY_BASE MESSAGE_KEY_keyBase
#define  KEY_VERSION MESSAGE_KEY_keyVersion
#define  KEY_GONEOFF MESSAGE_KEY_keyGoneoff
#define  KEY_TRANSMIT MESSAGE_KEY_keyTransmit
#define  KEY_AUTO_RESET MESSAGE_KEY_keyAutoReset
#define  KEY_SNOOZES MESSAGE_KEY_keySnoozes


enum CtrlValues {
  CTRL_TRANSMIT_DONE = 1,
  CTRL_VERSION_DONE = 2,
  CTRL_GONEOFF_DONE = 4,
  CTRL_DO_NEXT = 8,
  CTRL_SET_LAST_SENT = 16,
  CTRL_LAZARUS = 32,
  CTRL_SNOOZES_DONE = 64
};

typedef enum {
  IS_COMMS = 0,
  IS_RECORD,
  IS_IGNORE,
  IS_ALARM,
  IS_ALARM_RING,
  IS_BLUETOOTH,
  IS_EXPORT
} IconState;

enum ErrorCodes {
  ERR_ACCEL_DATA_SERVICE_SUBSCRIBE_DEAD = 1,
  ERR_ACCEL_DATA_SERVICE_SUBSCRIBE_STUCK_VIBE = 2
};

/*
 * Thresholds
 */
enum Thresholds {
    AWAKE_ABOVE = 1000,
    LIGHT_ABOVE = 120 
};

#define MAX_ICON_STATE 8

#define PERSIST_MEMORY_KEY 12121
#define PERSIST_CONFIG_KEY 12122
#define PERSIST_PRESET_KEY 12123
#define PERSIST_CHART_KEY 12124
#define PERSIST_MEMORY_MS (5*60*1000)
#define PERSIST_CONFIG_MS 30000
#define SHORT_RETRY_MS 200
#define LONG_RETRY_MS 60000
#define NOTICE_DISPLAY_MS 7000
#define CHART_DISPLAY_MS (5*60*1000)
#define FIVE_MINUTES_MS (5*60*1000)
#define TEN_SECONDS_MS (10*1000)
#define HALF_SECOND_MS (500)
#define COMPLETE_OUTSTANDING_MS (15*1000)
#define FIVE_MINUTES (5*60)
#define FIVE_SECONDS 5
#define VERSION_SEND_INTERVAL_MS (1000)
#define VERSION_SEND_SLOW_INTERVAL_MS (60*1000)

#define LIMIT 60
#define DIVISOR 600
#define MINS_PER_SEGMENT 10

#define MINS_IN_DAY 1440

#define TWENTY_FOUR_HOURS_IN_SECONDS (24*60*60)
#define ELEVEN_HOURS_IN_SECONDS (11*60*60)
#define WAKEUP_AUTO_RESTART 1
#define WAKEUP_FOR_TRANSMIT 2
#define WAKEUP_LAZARUS 3
#define ONE_MINUTE 60

#define EARLY_PRESET 0
#define MEDIUM_PRESET 1
#define LATE_PRESET 2

// Change INTERNAL_VER only if the InternalData struct changes
#define INTERNAL_VER 44
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
  bool stopped;
  uint8_t snoozes;
  bool snoozes_sent;
} InternalData;

// Change the CONFIG_VER only if the ConfigData struct changes
#define CONFIG_VER 42
typedef struct {
  uint8_t config_ver;
  bool unusedA;
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
  time_t unusedB;
} ConfigData;

typedef struct {
  BitmapLayer *layer;
  GBitmap *bitmap;
} BitmapLayerComp;

// Version from App Info stuff (undocumented)
#define VERSION_EXTERNAL extern const PebbleProcessInfo __pbl_app_info
#define VERSION_MAJOR (__pbl_app_info.process_version.major)
#define VERSION_MINOR (__pbl_app_info.process_version.minor)
#define APP_NAME (__pbl_app_info.name)

// Externals
ConfigData *get_config_data();
InternalData *get_internal_data();
Layer * macro_layer_create(GRect frame, Layer *parent, LayerUpdateProc update_proc);
TextLayer* macro_text_layer_create(GRect frame, Layer *parent, GColor tcolor, GColor bcolor, GFont font, GTextAlignment text_alignment);
bool get_icon(IconState icon);
bool is_animation_complete();
bool is_doing_powernap();
bool is_monitoring_sleep();
bool is_notice_showing();
char* am_pm_text(uint8_t hour);
#ifdef PBL_COLOR
GColor bar_color(uint16_t height);
#endif
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
void close_morpheuz();
#ifndef PBL_PLATFORM_APLITE
void copy_time_range_into_field(char *field, size_t fsize, uint8_t fromhr, uint8_t frommin, uint8_t tohr, uint8_t tomin);
#endif
void copy_alarm_time_range_into_field(char *field, size_t fsize);
void fire_alarm();
void hide_notice_layer(void *data);
void icon_bar_update_callback(Layer *layer, GContext *ctx);
void init_morpheuz();
void lazarus();
void macro_bitmap_layer_change_resource(BitmapLayerComp *comp, uint32_t new_resource_id);
void macro_bitmap_layer_create(BitmapLayerComp *comp, GRect frame, Layer *parent, uint32_t resource_id, bool visible);
void macro_bitmap_layer_destroy(BitmapLayerComp *comp);
void manual_shutdown_request();
void morpheuz_load(Window *window);
void morpheuz_load_standard_postamble();
void morpheuz_unload(Window *window);
void open_comms();
void post_init_hook(void *data);
void power_nap_check(uint16_t biggest);
void power_nap_countdown();
void power_nap_reset();
void progress_layer_update_callback(Layer *layer, GContext *ctx);
void read_config_data();
void read_internal_data();
void resend_all_data(bool invoked_by_change_of_time);
void reset_sleep_period();
void revive_clock_on_movement(uint16_t last_movement);
void save_config_data(void *data);
void save_internal_data();
void server_processing(uint16_t biggest);
void set_error_code(uint8_t new_error_code);
void set_icon(bool enabled, IconState icon);
void set_ignore_on_current_time_segment();
void set_next_wakeup();
void set_progress();
void set_smart_status();
void set_smart_status_on_screen(bool smart_alarm_on, char *special_text);
void show_alarm_visuals(bool value);
void show_menu();
void show_notice(uint32_t resource_id);
void show_preset_menu();
void show_set_alarm();
void snooze_alarm();
void toggle_power_nap();
void trigger_config_save();
void wakeup_init();
void wakeup_toggle();

#ifdef VOICE_SUPPORTED
void set_using_preset(uint8_t no);
void voice_control();
void tidy_voice();
bool is_voice_system_active();
void show_notice_with_message(uint32_t resource_id, char *message);
void voice_system_inactive();
void copy_end_time_into_field(char *field, size_t fsize);
#endif

#ifdef CACHE_ICONS
void init_icon_cache();
void destroy_icon_cache();
#else
#define init_icon_cache() 
#define destroy_icon_cache() 
#endif

#ifdef ENABLE_CHART_VIEWER
  void store_chart_data();
  void show_chart();
  bool is_chart_showing();
  void chart_load(Window *window);
  void chart_unload(Window *window);
#else
  #define store_chart_data()
  #define show_chart()
  #define is_chart_showing() false
#endif

#endif /* MORPHEUZ_H_ */
