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

#ifdef PBL_RECT

// Constants
#define BED_FINISH GRect(8, 17, 127, 70)
#define SLEEPER_FINISH GRect(25, 24, 110, 29)
#define HEAD_FINISH GRect(25, 24, 19, 16)
#define TEXT_FINISH GRect(26, 72, 92, 15)
#define BLOCK_FINISH GRect(0,169,144,78)
#define BED_START GRect(-144-8, 17, 127, 70)
#define SLEEPER_START GRect(25+144, 24, 110, 29)
#define HEAD_START GRect(25, -24, 19, 16)
#define TEXT_START GRect(26, 25, 92, 15)
#define BLOCK_START GRect(0,91,144,78)

// Private
static BitmapLayerComp logo_bed;
static BitmapLayerComp logo_sleeper;
static BitmapLayerComp logo_text;
static BitmapLayerComp logo_head;
static TextLayer *block_layer;
static struct PropertyAnimation *animations[MAX_ANIMATIONS];
#ifdef PBL_COLOR
static uint8_t text_color_count = 0;
#endif

#ifdef PBL_SDK_2
static InverterLayer *full_inverse_layer;
#endif

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
  

static void animation_stopped(Animation *animation, bool finished, void *data);

/*
 * Set up a single animation
 */
static void build_an_animate(Layer *layer, GRect *start, GRect *finish, uint32_t duration, uint8_t id) {
  animations[id] = property_animation_create_layer_frame(layer, start, finish);
  animation_set_duration((Animation*) animations[id], duration);
  animation_set_handlers((Animation*) animations[id], (AnimationHandlers ) { .stopped = (AnimationStoppedHandler) animation_stopped, }, NULL /* callback data */);
  animation_schedule((Animation*) animations[id]);
}

/*
 * End of initial animation sequence - occurs 4 then 5 times
 */
static void animation_stopped(Animation *animation, bool finished, void *data) {
#ifdef PBL_SDK_2
  animation_unschedule(animation);
  animation_destroy(animation);
#endif
  animation_count++;
  if (animation_count == 4) {
    light_enable_interaction();
    build_an_animate(bitmap_layer_get_layer_jf(logo_head.layer), &HEAD_START, &HEAD_FINISH, ANIMATE_HEAD_DURATION, LOGO_HEAD_ANIMATION);
  } else if (animation_count == 5) {
    analogue_visible(get_config_data()->analogue, true);
  }
}

/*
 * Start title animation sequence
 */
static void start_animate(void *data) {
  light_enable_interaction();

  build_an_animate(bitmap_layer_get_layer_jf(logo_bed.layer), &BED_START, &BED_FINISH, ANIMATE_MAIN_DURATION, LOGO_BED_ANIMATION);
  build_an_animate(bitmap_layer_get_layer_jf(logo_sleeper.layer), &SLEEPER_START, &SLEEPER_FINISH, ANIMATE_MAIN_DURATION, LOGO_SLEEPER_ANIMATION);
  build_an_animate(bitmap_layer_get_layer_jf(logo_text.layer), &TEXT_START, &TEXT_FINISH, ANIMATE_MAIN_DURATION, LOGO_TEXT_ANIMATION);
  build_an_animate(text_layer_get_layer_jf(block_layer), &BLOCK_START, &BLOCK_FINISH, ANIMATE_MAIN_DURATION, BLOCK_ANIMATION);
  text_layer_destroy(version_text);
  text_layer_set_text(block_layer, "");
}

#ifdef PBL_COLOR
/*
 * Move copyright text through color cycle
 */
static void text_color_cycle(void *data) {
  if (text_color_count >= 8) {
    start_animate(NULL);
    return;
  }
  text_layer_set_text_color(block_layer, bar_color(text_color_count));
  text_color_count++;
  app_timer_register(INTER_TEXT_COLOR_MS, text_color_cycle, NULL);
}
#endif

/*
 * Load on window load
 */
EXTFN void morpheuz_load(Window *window) {
  
  animation_count = 0;
  
  window_set_background_color(window, BACKGROUND_COLOR);

  notice_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_16));
  time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_38));

  Layer *window_layer = window_get_root_layer(window);

  powernap_layer = macro_text_layer_create(GRect(5, -3, 20, 19), window_layer, GColorWhite, BACKGROUND_COLOR, fonts_get_system_font(FONT_KEY_GOTHIC_14), GTextAlignmentCenter);

  macro_bitmap_layer_create(&logo_bed, BED_START, window_layer, RESOURCE_ID_IMAGE_LOGO_BED, true);

  macro_bitmap_layer_create(&logo_sleeper, SLEEPER_START, window_layer, RESOURCE_ID_IMAGE_LOGO_SLEEPER, true);

  macro_bitmap_layer_create(&logo_text, TEXT_START, window_layer, RESOURCE_ID_IMAGE_LOGO_TEXT, true);

  macro_bitmap_layer_create(&logo_head, HEAD_START, window_layer, RESOURCE_ID_IMAGE_LOGO_HEAD, true);

  version_text = macro_text_layer_create(GRect(26, 43, 92, 30), window_layer, GColorWhite, GColorClear, notice_font, GTextAlignmentCenter);
  text_layer_set_text(version_text, VERSION_TXT);
  
  #ifdef PBL_COLOR
      text_time_shadow_layer = macro_text_layer_create(GRect(4, 113, 144, 42), window_layer, GColorOxfordBlue, BACKGROUND_COLOR, time_font, GTextAlignmentCenter);
  #endif

  text_time_layer = macro_text_layer_create(GRect(0, 109, 144, 42), window_layer, GColorWhite, GColorClear, time_font, GTextAlignmentCenter);

  text_date_smart_alarm_range_layer = macro_text_layer_create(GRect(0, 86, 144, 31), window_layer, GColorWhite, BACKGROUND_COLOR, fonts_get_system_font(FONT_KEY_GOTHIC_24), GTextAlignmentCenter);

  icon_bar = layer_create(GRect(26, ICON_TOPS, ICON_BAR_WIDTH, 12));
  layer_set_update_proc(icon_bar, &icon_bar_update_callback);
  layer_add_child(window_layer, icon_bar);

  BatteryChargeState initial = battery_state_service_peek();
  battery_level = initial.charge_percent;
  battery_plugged = initial.is_plugged;
  bluetooth_state_handler(bluetooth_connection_service_peek());

  progress_layer = layer_create(GRect(11, 157, 121, 9));
  layer_set_update_proc(progress_layer, &progress_layer_update_callback);
  layer_add_child(window_layer, progress_layer);

  block_layer = macro_text_layer_create(BLOCK_START, window_layer, COPYRIGHT_COLOR, BACKGROUND_COLOR, notice_font, GTextAlignmentCenter);
  text_layer_set_text(block_layer, COPYRIGHT);

  analogue_window_load(window);
  
  macro_bitmap_layer_create(&alarm_button_top, GRect(114, 17, 30, 30), window_layer, RESOURCE_ID_BUTTON_ALARM_TOP, false);
  macro_bitmap_layer_create(&alarm_button_button, GRect(114, 120, 30, 30), window_layer, RESOURCE_ID_BUTTON_ALARM_BOTTOM, false);
  
#ifdef PBL_SDK_2
  full_inverse_layer = inverter_layer_create(GRect(0, 0, 144, 168));
  layer_add_child(window_layer, inverter_layer_get_layer_jf(full_inverse_layer));
  layer_set_hidden(inverter_layer_get_layer_jf(full_inverse_layer), true);
#endif

  read_internal_data();
  read_config_data();

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

  battery_state_service_subscribe(&battery_state_handler);

  bluetooth_connection_service_subscribe(bluetooth_state_handler);

#ifndef PBL_COLOR
  invert_screen();
#endif

  init_morpheuz();

  set_icon(get_internal_data()->transmit_sent, IS_EXPORT);

  light_enable_interaction();

  #ifndef PBL_COLOR
    app_timer_register(PRE_ANIMATE_DELAY, start_animate, NULL);
  #else
    app_timer_register(INTER_TEXT_COLOR_MS, text_color_cycle, NULL);
  #endif    
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

  // Save space by not clearing up on close on aplite. Feels bad, but so do crashes for no heap.
  #ifndef PBL_PLATFORM_APLITE 
  #ifdef PBL_SDK_2
    inverter_layer_destroy(full_inverse_layer);
  #endif
  
  analogue_window_unload();

  layer_destroy(progress_layer);
  layer_destroy(icon_bar);

  text_layer_destroy(text_time_layer);
  text_layer_destroy(text_date_smart_alarm_range_layer);
  text_layer_destroy(powernap_layer);
  
  #ifdef PBL_COLOR
    text_layer_destroy(text_time_shadow_layer);
  #endif

  macro_bitmap_layer_destroy(&logo_bed);
  macro_bitmap_layer_destroy(&logo_sleeper);
  macro_bitmap_layer_destroy(&logo_text);
  macro_bitmap_layer_destroy(&logo_head);
  macro_bitmap_layer_destroy(&alarm_button_top);
  macro_bitmap_layer_destroy(&alarm_button_button);

  fonts_unload_custom_font(time_font);
  fonts_unload_custom_font(notice_font);
  
    #ifdef VOICE_SUPPORTED
      tidy_voice();
    #endif
  
  #endif
}

#ifndef PBL_COLOR
/*
 * Invert screen
 */
EXTFN void invert_screen() {
  layer_set_hidden(inverter_layer_get_layer_jf(full_inverse_layer), !get_config_data()->invert);
}
#endif

/*
 * Hide the bed when making the analogue face visible
 */
EXTFN void bed_visible(bool value) {
  layer_set_hidden(bitmap_layer_get_layer_jf(logo_bed.layer), !value);
  layer_set_hidden(bitmap_layer_get_layer_jf(logo_head.layer), !value);
}

#endif
