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

#include "pebble.h"
#include "morpheuz.h"
#include "language.h"

#ifdef PBL_ROUND

// Constants
#define HOUR_RADIUS 8
#define HOUR_COLOR GColorVividCerulean
#define MINUTE_RADIUS 6
#define MINUTE_COLOR GColorIcterine 
  
// Private
static BitmapLayerComp round_background;
static Layer *analogue_time_layer;
static int current_hour, current_minutes; 

// Shared with rootui, rectui, roundui and primary_window with main
extern Window *primary_window;
extern Layer *icon_bar;
extern TextLayer *text_date_smart_alarm_range_layer;
extern uint8_t animation_count;
extern TextLayer *powernap_layer;
#ifdef PBL_COLOR
extern TextLayer *text_time_shadow_layer;
#endif 
extern TextLayer *text_time_layer;
extern uint8_t battery_level;
extern bool battery_plugged;
extern BitmapLayerComp alarm_button_top;
extern BitmapLayerComp alarm_button_button;
extern Layer *progress_layer;
extern GFont notice_font;
extern GFont time_font;
extern TextLayer *version_text;

/*
 * Not currently supporting a base time indicator on the chalk clockface. Might do in the future hence this dummy function.
 */
void analogue_set_base(time_t base) {
}

/*
 * Not currently supporting a radial progress indicator on the chalk clockface. Might do in the future hence this dummy function.
 */
void analogue_set_progress(uint8_t progress_level_in) {
}

/*
 * Only one real face on the chalk version, so this isn't used. Might change in the future
 */
void analogue_visible(bool visible, bool call_post_init) {
}

/*
 * Don't currently show smart times radially on the chalk watchface
 */
void analogue_set_smart_times() {
}

/*
 * Process analogue clock tick
 */
void analogue_minute_tick() {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  current_hour = t->tm_hour;
  current_minutes = t->tm_min;
  layer_mark_dirty(analogue_time_layer);
}

/*
 * Convert hours into an angle (include minutes, 'cos mechanical clocks would)
 */
static int32_t get_angle_for_hour(int hour, int minute) {
  return ((hour * 360) / 12) + ((minute * 30) / 60);
}

/*
 * Convert minutes into an angle
 */
static int32_t get_angle_for_minute(int minute) {
  return (minute * 360) / 60;
}

static void layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GRect frame = grect_inset(bounds, GEdgeInsets(11));

  // 12 hours only, with a minimum size
  current_hour -= (current_hour > 12) ? 12 : 0;
  
  // Hour blobby
  int hour_angle = get_angle_for_hour(current_hour, current_minutes);
  GPoint posh = gpoint_from_polar(frame, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(hour_angle));
  graphics_context_set_fill_color(ctx, HOUR_COLOR);
  graphics_fill_circle(ctx, posh, HOUR_RADIUS);
  
  // Minute blobby
  int minute_angle = get_angle_for_minute(current_minutes);
  GPoint posm = gpoint_from_polar(frame, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(minute_angle));
  graphics_context_set_fill_color(ctx, MINUTE_COLOR);
  graphics_fill_circle(ctx, posm, MINUTE_RADIUS);

}

/*
 * Loading of screen complete
 */
static void load_complete(void *data) {
   text_layer_destroy(version_text);
   app_timer_register(250, post_init_hook, NULL);
}

/*
 * Load on window load
 */
EXTFN void morpheuz_load(Window *window) {
  
  animation_count = 0;
  
  window_set_background_color(window, BACKGROUND_COLOR);

  notice_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_16));
  time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_38));

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  int16_t width = bounds.size.w;
  
  macro_bitmap_layer_create(&round_background, bounds, window_layer, RESOURCE_ID_IMAGE_ROUND_BACKGROUND, true);

  powernap_layer = macro_text_layer_create(GRect(122, 125, 20, 19), window_layer, GColorWhite, BACKGROUND_COLOR, fonts_get_system_font(FONT_KEY_GOTHIC_14), GTextAlignmentCenter);

  version_text = macro_text_layer_create(GRect(40, 125, 20, 19), window_layer, GColorWhite, GColorClear, fonts_get_system_font(FONT_KEY_GOTHIC_14), GTextAlignmentCenter);
  text_layer_set_text(version_text, VERSION_TXT);
  
  text_date_smart_alarm_range_layer = macro_text_layer_create(GRect(15, 74, 150, 25), window_layer, GColorWhite, BACKGROUND_COLOR, fonts_get_system_font(FONT_KEY_GOTHIC_24), GTextAlignmentCenter);
 
  #ifdef PBL_COLOR
      text_time_shadow_layer = macro_text_layer_create(GRect(2, 36, width, 44), window_layer, GColorOxfordBlue, GColorClear, time_font, GTextAlignmentCenter);
  #endif

  text_time_layer = macro_text_layer_create(GRect(0, 34, width, 44), window_layer, GColorWhite, GColorClear, time_font, GTextAlignmentCenter);
  
  icon_bar = layer_create(GRect(47, ICON_TOPS, ICON_BAR_WIDTH, 12));
  layer_set_update_proc(icon_bar, &icon_bar_update_callback);
  layer_add_child(window_layer, icon_bar);

  BatteryChargeState initial = battery_state_service_peek();
  battery_level = initial.charge_percent;
  battery_plugged = initial.is_plugged;
  bluetooth_state_handler(bluetooth_connection_service_peek());

  progress_layer = layer_create(GRect(29, 105, 121, 9));
  layer_set_update_proc(progress_layer, &progress_layer_update_callback);
  layer_add_child(window_layer, progress_layer);
  
  analogue_time_layer = layer_create(bounds);
  layer_set_update_proc(analogue_time_layer, layer_update_proc);
  layer_add_child(window_layer, analogue_time_layer);

  macro_bitmap_layer_create(&alarm_button_top, GRect(138, 39, 30, 30), window_layer, RESOURCE_ID_BUTTON_ALARM_TOP, false);
  macro_bitmap_layer_create(&alarm_button_button, GRect(138, 108, 30, 30), window_layer, RESOURCE_ID_BUTTON_ALARM_BOTTOM, false);
    
  read_internal_data();
  read_config_data();

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

  battery_state_service_subscribe(&battery_state_handler);

  bluetooth_connection_service_subscribe(bluetooth_state_handler);

  init_morpheuz();

  set_icon(get_internal_data()->transmit_sent, IS_EXPORT);
  
  app_timer_register(PRE_ANIMATE_DELAY, load_complete, NULL);
  
}
  
/*
 * Shutdown
 */
EXTFN void morpheuz_unload(Window *window) {
  
  hide_notice_layer(NULL);

  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  accel_data_service_unsubscribe();

  save_config_data(NULL);
  save_internal_data();
 
  layer_destroy(progress_layer);
  layer_destroy(icon_bar);
  
  layer_destroy(analogue_time_layer);

  text_layer_destroy(text_time_layer);
  text_layer_destroy(text_date_smart_alarm_range_layer);
  text_layer_destroy(powernap_layer);
  
  text_layer_destroy(text_time_shadow_layer);

  macro_bitmap_layer_destroy(&alarm_button_top);
  macro_bitmap_layer_destroy(&alarm_button_button);

  fonts_unload_custom_font(time_font);
  fonts_unload_custom_font(notice_font);
  
}

#endif