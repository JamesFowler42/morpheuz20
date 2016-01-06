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

#include "pebble.h"
#include "morpheuz.h"
#include "language.h"
#include "rootui.h"

// Icons in the bitmap cache (if we are doing this)
typedef enum {
  BMP_CACHE_BATTERY_ICON,
  BMP_CACHE_BATTERY_CHARGE,
  BMP_CACHE_COMMS_ICON,
  BMP_CACHE_BLUETOOTH_ICON,
  BMP_CACHE_ICON_RECORD,
  BMP_CACHE_ALARM_RING_ICON,
  BMP_CACHE_ALARM_ICON,
  BMP_CACHE_IGNORE,
  BMP_CACHE_EXPORT,
  BMP_CACHE_TOP
} BmpCache;

// A bitmap cache entry
typedef struct {
  GBitmap *bitmap;
} BmpCacheEntry;

// Only if we are caching icons do we declare this
#ifdef CACHE_ICONS
static BmpCacheEntry bmp_cache_entries[BMP_CACHE_TOP];
#endif
 
// Private  
static bool icon_state[MAX_ICON_STATE];
static uint8_t previous_mday = 255;
static char powernap_text[3];

// Shared with rootui, rectui, roundui, primary_window with main and notice_font with noticewindows
UiCommon ui;

// Shared with menu, rootui and presets 
char date_text[DATE_FORMAT_LEN] = "";

#ifdef CACHE_ICONS
/*
 * Set up the icon cache - make sure NULL for all entries
 */
EXTFN void init_icon_cache() {
  for (uint8_t i=0; i < BMP_CACHE_TOP; i++) {
    bmp_cache_entries[i].bitmap = NULL;
  }
}

/*
 * Get a bitmap from cache, failing that from resources
 */
static GBitmap* gbitmap_create_with_resource_cache(uint32_t resource_id, BmpCache cache_id) {
  if (bmp_cache_entries[cache_id].bitmap == NULL) {
    bmp_cache_entries[cache_id].bitmap = gbitmap_create_with_resource(resource_id);
  }
  return bmp_cache_entries[cache_id].bitmap;
}

/*
 * When caching we don't get rid of the bitmap
 */
#define gbitmap_destroy_cache(bitmap)

/*
 * Remove all the allocated icons
 */
EXTFN void destroy_icon_cache() {
  for (uint8_t i=0; i < BMP_CACHE_TOP; i++) {
    if (bmp_cache_entries[i].bitmap != NULL) {
      gbitmap_destroy(bmp_cache_entries[i].bitmap);
    }
  }
}
#else
#define gbitmap_create_with_resource_cache(resource_id, cache_id) gbitmap_create_with_resource(resource_id)
#define gbitmap_destroy_cache(bitmap) gbitmap_destroy(bitmap)
#endif

/*
 * Perform the clock update
 */
static void update_clock() {
  static char time_text[6];
  clock_copy_time_string(time_text, sizeof(time_text));
  if (time_text[4] == ' ')
    time_text[4] = '\0';
  text_layer_set_text(ui.text_time_layer, time_text);
  #ifdef PBL_COLOR
     text_layer_set_text(ui.text_time_shadow_layer, time_text); 
  #endif
  analogue_minute_tick();
}

/**
 * Back button single click handler
 */
static void back_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Stop accidental closure of Morpheuz by defining this
  // Bring clock up to date if a button is pressed
  // Only if we're recording or running powernap
  if (is_monitoring_sleep()) {
    manual_shutdown_request();
  } else {
    close_morpheuz();  
  }
}

/*
 * Single click handler on down button
 */
static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Make the snooze and the cancel buttons the same way around as the default alarm app
  cancel_alarm();
}

/*
 * Single click handler on select button
 */
static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Bring clock up to date if a button is pressed
  if (!is_notice_showing())
    show_menu();
}

/*
 * Single click handler on up button
 */
static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Make the snooze and the cancel buttons the same way around as the default alarm app
  snooze_alarm();
}

#ifdef VOICE_SUPPORTED
/*
 * Long click handler on select button
 */
static void select_click_handler_long(ClickRecognizerRef recognizer, void *context) {
  voice_control();
}
#endif

/*
 * Button config
 */
static void click_config_provider(Window *window) {
  window_single_click_subscribe(BUTTON_ID_BACK, back_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);
  #ifdef VOICE_SUPPORTED
    window_long_click_subscribe(BUTTON_ID_SELECT, 0, select_click_handler_long, NULL);
  #endif
}

/**
 * Indicate if title animation complete
 */
EXTFN bool is_animation_complete() {
  return ui.animation_count == 6;
}

/*
 * Stuff we only allow after we've gone through the normal pre-amble
 */
EXTFN void post_init_hook(void *data) {
  wakeup_init();
  ui.animation_count = 6; // Make it 6 so we consider is_animation_complete() will return true
  layer_mark_dirty(ui.icon_bar);
 
  // Set click provider
  window_set_click_config_provider(ui.primary_window, (ClickConfigProvider) click_config_provider);
}

/*
 * Bar chart color based on height of bar. Would normally do this with a constant array, but GColorXXXX aren't constants apparently.
 */
#ifdef PBL_COLOR
EXTFN GColor bar_color(uint16_t height) {
  if (height == 0) return GColorBlue; 
  if (height == 1) return GColorBlueMoon;
  if (height == 2) return GColorPictonBlue;
  if (height == 3) return GColorVividCerulean;
  if (height == 4) return GColorMalachite;
  if (height == 5) return GColorBrightGreen;
  if (height == 6) return GColorSpringBud;
  if (height == 7) return GColorYellow;
  return GColorPastelYellow;
}
#endif
  
  /*
 * Set the icon state for any icon
 */
EXTFN void set_icon(bool enabled, IconState icon) {
  if (enabled != icon_state[icon]) {
    icon_state[icon] = enabled;
    layer_mark_dirty(ui.icon_bar);
  }
}

/*
 * Get the icon state
 */
EXTFN bool get_icon(IconState icon) {
  return icon_state[icon];
}

/*
 * Are we monitoring sleep (recording or powernap)? 
 * Also now includes case where we have stopped recording and are ringing the alarm (including snoozed alarm)
 */
EXTFN bool is_monitoring_sleep() {
  return get_icon(IS_RECORD) || is_doing_powernap() || get_icon(IS_ALARM_RING);
}

/*
 * Set the smart alarm status details
 */
EXTFN void set_smart_status_on_screen(bool smart_alarm_on, char *special_text) {
  set_icon(smart_alarm_on, IS_ALARM);
  #ifndef TESTING_MEMORY_LEAK
  if (smart_alarm_on)
    text_layer_set_text(ui.text_date_smart_alarm_range_layer, special_text);
  else
    text_layer_set_text(ui.text_date_smart_alarm_range_layer, date_text);
  #endif
}

/*
 * Build an icon
 */
static void paint_icon(GContext *ctx, int *running_horizontal, int width, uint32_t resource_id, BmpCache cacheId) {
  GBitmap *bitmap = gbitmap_create_with_resource_cache(resource_id, cacheId);
  *running_horizontal -= width + ICON_PAD;
  graphics_draw_bitmap_in_rect(ctx, bitmap, GRect(*running_horizontal, 0, width, 12));
  gbitmap_destroy_cache(bitmap);
}

/*
 * Battery icon callback handler (called via icon bar update now)
 */
static void battery_layer_update_callback(Layer *layer, GContext *ctx, int *running_horizontal) {

  #ifdef PBL_COLOR
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
  #else
    graphics_context_set_compositing_mode(ctx, GCompOpAssign);
  #endif

  if (!ui.battery_plugged) {
    paint_icon(ctx, running_horizontal, 24, RESOURCE_ID_BATTERY_ICON, BMP_CACHE_BATTERY_ICON);
    graphics_context_set_stroke_color(ctx, BACKGROUND_COLOR);
    #ifdef PBL_COLOR
      GColor b_color = BATTERY_BAR_COLOR;
      if (ui.battery_level <= 20) {
        b_color = BATTERY_BAR_COLOR_CRITICAL;
      } else if (ui.battery_level <= 40) {
        b_color = BATTERY_BAR_COLOR_WARN;
      }
      graphics_context_set_fill_color(ctx, b_color);
    #else
      graphics_context_set_fill_color(ctx, BATTERY_BAR_COLOR);
    #endif
    graphics_fill_rect(ctx, GRect(*running_horizontal + 7, 4, ui.battery_level / 9, 4), 0, GCornerNone);
  } else {
    paint_icon(ctx, running_horizontal, 24, RESOURCE_ID_BATTERY_CHARGE, BMP_CACHE_BATTERY_CHARGE);
  }
}

/*
 * Icon bar update handler
 */
EXTFN void icon_bar_update_callback(Layer *layer, GContext *ctx) {

  int running_horizontal = ICON_BAR_WIDTH;

  graphics_context_set_fill_color(ctx, BACKGROUND_COLOR);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorBlack);

#ifdef PBL_COLOR
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
#endif

  // Don't draw if we're currently doing
  if (!is_animation_complete())
    return;

  // Battery icon (always showing)
  battery_layer_update_callback(layer, ctx, &running_horizontal);
  running_horizontal += ICON_PAD_BATTERY;

  // Comms icon / Bluetooth icon
  if (icon_state[IS_COMMS] || icon_state[IS_BLUETOOTH]) {
    paint_icon(ctx, &running_horizontal, 9, icon_state[IS_COMMS] ? RESOURCE_ID_COMMS_ICON : RESOURCE_ID_BLUETOOTH_ICON,
               icon_state[IS_COMMS] ? BMP_CACHE_COMMS_ICON : BMP_CACHE_BLUETOOTH_ICON);
  }

  // Record icon
  if (icon_state[IS_RECORD]) {
    paint_icon(ctx, &running_horizontal, 10, RESOURCE_ID_ICON_RECORD, BMP_CACHE_ICON_RECORD);
  }

  // Alarm icon
  if (icon_state[IS_ALARM_RING] || icon_state[IS_ALARM]) {
    paint_icon(ctx, &running_horizontal, 12, icon_state[IS_ALARM_RING] ? RESOURCE_ID_ALARM_RING_ICON : RESOURCE_ID_ALARM_ICON,
              icon_state[IS_ALARM_RING] ? BMP_CACHE_ALARM_RING_ICON : BMP_CACHE_ALARM_ICON);
  }

  // Ignore icon
  if (icon_state[IS_IGNORE]) {
    paint_icon(ctx, &running_horizontal, 9, RESOURCE_ID_IGNORE, BMP_CACHE_IGNORE );
  }

  // Export icon
  if (icon_state[IS_EXPORT]) {
    paint_icon(ctx, &running_horizontal, 9, RESOURCE_ID_EXPORT, BMP_CACHE_EXPORT);
  }
}

/*
 * Battery state change
 */
EXTFN void battery_state_handler(BatteryChargeState charge) {
  ui.battery_level = charge.charge_percent;
  ui.battery_plugged = charge.is_plugged;
  layer_mark_dirty(ui.icon_bar);
}

/*
 * Progress line
 */
EXTFN void progress_layer_update_callback(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, BACKGROUND_COLOR);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);

  graphics_context_set_fill_color(ctx, BAR_CHART_MARKS);

  graphics_context_set_stroke_color(ctx, BAR_CHART_MARKS);

  for (uint8_t i = 0; i <= 120; i += 12) {
    graphics_draw_pixel(ctx, GPoint(i, 8));
    graphics_draw_pixel(ctx, GPoint(i, 7));
  }

  for (uint8_t i = 0; i <= get_internal_data()->highest_entry; i++) {
    if (!get_internal_data()->ignore[i]) {
      uint16_t height = get_internal_data()->points[i] / 500;
      uint8_t i2 = i * 2;
      #ifdef PBL_COLOR
          graphics_context_set_stroke_color(ctx, bar_color(height));
      #endif
      graphics_draw_line(ctx, GPoint(i2, 8 - height), GPoint(i2, 8));
    }
  }
}



/*
 * Process clockface
 */
EXTFN void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {

  #ifndef TESTING_MEMORY_LEAK
    // Only update the date if the day has changed
    if (tick_time->tm_mday != previous_mday) {
      strftime(date_text, sizeof(date_text), DATE_FORMAT, tick_time);
      if (!icon_state[IS_ALARM])
        text_layer_set_text(ui.text_date_smart_alarm_range_layer, date_text);
      previous_mday = tick_time->tm_mday;
    }
  #else
    // Use the line to show heap space in testing mode
    snprintf(date_text, sizeof(date_text), "%d", heap_bytes_free());
    text_layer_set_text(text_date_smart_alarm_range_layer, date_text);
  #endif

  // Perform all background processing
  every_minute_processing();

  // Do the power nap countdown
  power_nap_countdown();

  // Update the clock
  update_clock();
}

/*
 * Bluetooth connection status
 */
EXTFN void bluetooth_state_handler(bool connected) {
  set_icon(connected, IS_BLUETOOTH);
  if (!connected)
    set_icon(false, IS_COMMS); // because we only set comms state on NAK/ACK it can be at odds with BT state - do this else that is confusing
}

/*
 * Progress indicator position (1-54)
 */
EXTFN void set_progress() {
  if (!get_config_data()->analogue)
    layer_mark_dirty(ui.progress_layer);
}

/*
 * Set the power nap text for the display
 */
EXTFN void analogue_powernap_text(char *text) {
  strncpy(powernap_text, text, sizeof(powernap_text));
  text_layer_set_text(ui.powernap_layer, powernap_text);
}

/*
 * Show the alarm hint buttons and set icon
 */
EXTFN void show_alarm_visuals(bool value) {
  set_icon(value, IS_ALARM_RING);
  layer_set_hidden(bitmap_layer_get_layer_jf(ui.alarm_button_top.layer), !value);
  layer_set_hidden(bitmap_layer_get_layer_jf(ui.alarm_button_button.layer), !value);
}