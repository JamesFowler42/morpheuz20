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

#include "pebble.h"
#include "morpheuz.h"
#include "language.h"
#include "analogue.h"

static Window *window;

static Layer *progress_layer;
static Layer *battery_layer;

static TextLayer *text_date_layer;
static TextLayer *smart_alarm_range_layer;
static TextLayer *text_time_layer;
static TextLayer *version_text;
static TextLayer *block_layer;
static TextLayer *block_layer2;
static TextLayer *powernap_layer;

static BitmapLayerComp logo_bed;
static BitmapLayerComp logo_sleeper;
static BitmapLayerComp logo_text;
static BitmapLayerComp logo_head;
static BitmapLayerComp alarm_icon;
static BitmapLayerComp bluetooth_icon;
static BitmapLayerComp comms_icon;
static BitmapLayerComp record_icon;
static BitmapLayerComp alarm_ring;
static BitmapLayerComp ignore_icon;
static BitmapLayerComp activity_icon;
static BitmapLayerComp weekend;

static InverterLayer *full_inverse_layer;

static GBitmap *icon_battery;
static GBitmap *icon_battery_charge;

static GFont time_font;

static struct PropertyAnimation *animations[MAX_ANIMATIONS];

static uint8_t battery_level;
static bool battery_plugged;
static char powernap_text[3];
static uint8_t animation_count = 0;
static uint8_t previous_mday = 255;
static time_t last_clock_update;

char date_text[16] = "";
GFont notice_font;

/*
 * Boot up background process
 */
void start_worker() {
  AppWorkerResult result = app_worker_launch();
  if (result == APP_WORKER_RESULT_SUCCESS || result == APP_WORKER_RESULT_ALREADY_RUNNING) {
    layer_set_hidden(bitmap_layer_get_layer_jf(activity_icon.layer), false);
  }
  APP_LOG(APP_LOG_LEVEL_ERROR, "wlaunch %d", result);
}

/**
 * Close background process
 */
static void stop_worker() {
  bool running = app_worker_is_running();

  // Toggle running state
  if (running) {
    AppWorkerResult result = app_worker_kill();
    APP_LOG(APP_LOG_LEVEL_ERROR, "wkill %d", result);
  }
}

/**
 * Show worker icon
 */
static void show_worker() {
  layer_set_hidden(bitmap_layer_get_layer_jf(activity_icon.layer), !app_worker_is_running());
}

/*
 * Set the smart alarm status details
 */
void set_smart_status_on_screen(bool smart_alarm_on, char *special_text) {
  layer_set_hidden(bitmap_layer_get_layer_jf(alarm_icon.layer), !smart_alarm_on);
  layer_set_hidden(text_layer_get_layer_jf(smart_alarm_range_layer), !smart_alarm_on);
  if (smart_alarm_on)
    text_layer_set_text(smart_alarm_range_layer, special_text);
}

/*
 * Battery icon callback handler
 */
static void battery_layer_update_callback(Layer *layer, GContext *ctx) {

  graphics_context_set_compositing_mode(ctx, GCompOpAssign);
  if (!battery_plugged) {
    graphics_draw_bitmap_in_rect(ctx, icon_battery, GRect(0, 0, 24, 12));
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, GRect(7, 4, battery_level / 9, 4), 0, GCornerNone);
  } else {
    graphics_draw_bitmap_in_rect(ctx, icon_battery_charge, GRect(0, 0, 24, 12));
  }
}

/*
 * Battery state change
 */
static void battery_state_handler(BatteryChargeState charge) {
  battery_level = charge.charge_percent;
  battery_plugged = charge.is_plugged;
  layer_mark_dirty(battery_layer);
}

/*
 * Progress line
 */
static void progress_layer_update_callback(Layer *layer, GContext *ctx) {
  //graphics_context_set_compositing_mode(ctx, GCompOpAssign);
  //graphics_draw_bitmap_in_rect(ctx, image_progress, GRect(0, 0, 131, 5));
  //graphics_draw_bitmap_in_rect(ctx, image_progress_full, GRect(0, 0, progress_level * 131 / LIMIT, 5));
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);

  graphics_context_set_fill_color(ctx, GColorWhite);

  graphics_context_set_stroke_color(ctx, GColorWhite);

  for (uint8_t i = 0; i <= 108; i+=12) {
    graphics_draw_pixel(ctx, GPoint(i,8));
    graphics_draw_pixel(ctx, GPoint(i,7));
  }

  for (uint8_t i = 0; i <= get_internal_data()->highest_entry; i++) {
    if (!get_internal_data()->ignore[i]) {
      uint16_t height = get_internal_data()->points[i]/500;
      uint8_t i2 = i*2;
      graphics_draw_line(ctx, GPoint(i2,8-height), GPoint(i2,8));
    }
  }
}

/*
 * Perform the clock update
 */
static void update_clock() {
  static char time_text[6];
  clock_copy_time_string(time_text, sizeof(time_text));
  if (time_text[4] == ' ')
    time_text[4] = '\0';
  text_layer_set_text(text_time_layer, time_text);
  analogue_minute_tick();
  last_clock_update = time(NULL);
}

/*
 * Process clockface
 */
static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {

  // Only update the date if the day has changed
  if (tick_time->tm_mday != previous_mday) {
    strftime(date_text, sizeof(date_text), "%B %e", tick_time);
    text_layer_set_text(text_date_layer, date_text);
    previous_mday = tick_time->tm_mday;
  }

  // Perform all background processing
  uint16_t last_movement;
  if (is_animation_complete()) {
    last_movement = every_minute_processing();
    show_worker();
  } else {
    last_movement = CLOCK_UPDATE_THRESHOLD;
  }

  // Do the power nap countdown
  power_nap_countdown();

  // Only update the clock every five minutes unless awake
  if (last_movement >= CLOCK_UPDATE_THRESHOLD || (tick_time->tm_min % 5 == 0)) {
    update_clock();
  }
}

/*
 * Display the clock on movement (ensures if you start moving the clock is up to date)
 * Also fired from button press
 */
void revive_clock_on_movement(uint16_t last_movement) {

  if (last_movement >= CLOCK_UPDATE_THRESHOLD) {
    time_t now = time(NULL);
    if ((now - last_clock_update) > 60) {
      update_clock();
    }
  }

}

/*
 * Bluetooth connection status
 */
static void bluetooth_state_handler(bool connected) {
  layer_set_hidden(bitmap_layer_get_layer_jf(bluetooth_icon.layer), !connected);
  if (!connected)
    show_comms_state(false); // because we only set comms state on NAK/ACK it can be at odds with BT state - do this else that is confusing
}

/*
 * Comms connection status
 */
void show_comms_state(bool connected) {
  layer_set_hidden(bitmap_layer_get_layer_jf(comms_icon.layer), !connected);
}

/*
 * Ignore status
 */
void show_ignore_state(bool ignore) {
  layer_set_hidden(bitmap_layer_get_layer_jf(ignore_icon.layer), !ignore);
}

/*
 * Are we in ignore mode or not
 */
bool get_ignore_state() {
  return !layer_get_hidden(bitmap_layer_get_layer_jf(ignore_icon.layer));
}

/*
 * Alarm status icon
 */
void set_alarm_icon(bool show_icon) {
  layer_set_hidden(bitmap_layer_get_layer_jf(alarm_ring.layer), !show_icon);
}

/*
 * Progress indicator position (1-54)
 */
void set_progress() {
  if (!get_config_data()->analogue)
    layer_mark_dirty(progress_layer);
}

/*
 * Show record
 */
void show_record(bool recording) {
  layer_set_hidden(bitmap_layer_get_layer_jf(record_icon.layer), !recording);
}

/*
 * Stuff we only allow after we've gone through the normal pre-amble
 */
void post_init_hook(void *data) {
  wakeup_init();
  animation_count++; // Make it 6 so we consider is_animation_complete() will return true
}

static void animation_stopped(Animation *animation, bool finished, void *data);

/*
 * Set up a single animation
 */
static void build_an_animate(Layer *layer, GRect *start, GRect *finish, uint32_t duration, uint8_t id) {
  animations[id] = property_animation_create_layer_frame(layer, start, finish);
  animation_set_duration((Animation*) animations[id], duration);
  animation_set_handlers((Animation*) animations[id], (AnimationHandlers ) { .stopped = (AnimationStoppedHandler) animation_stopped, }, NULL /* callback data */);
}

/*
 * End of initial animation sequence - occurs 4 then 5 times
 */
static void animation_stopped(Animation *animation, bool finished, void *data) {
  animation_unschedule(animation);
  animation_destroy(animation);
  animation_count++;
  if (animation_count == 4) {
    light_enable_interaction();
    text_layer_destroy(block_layer2);
    build_an_animate(bitmap_layer_get_layer_jf(logo_head.layer), &HEAD_START, &HEAD_FINISH, 500, LOGO_HEAD_ANIMATION);
    animation_schedule((Animation*) animations[LOGO_HEAD_ANIMATION]);
  } else if (animation_count == 5) {
    analogue_visible(get_config_data()->analogue, true);
  }
}

/**
 * Indicate if title animation complete
 */
bool is_animation_complete() {
  return animation_count == 6;
}

/*
 * Start title animation sequence
 */
static void start_animate(void *data) {
  light_enable_interaction();

  build_an_animate(bitmap_layer_get_layer_jf(logo_bed.layer), &BED_START, &BED_FINISH, 1000, LOGO_BED_ANIMATION);
  build_an_animate(bitmap_layer_get_layer_jf(logo_sleeper.layer), &SLEEPER_START, &SLEEPER_FINISH, 1000, LOGO_SLEEPER_ANIMATION);
  build_an_animate(bitmap_layer_get_layer_jf(logo_text.layer), &TEXT_START, &TEXT_FINISH, 1000, LOGO_TEXT_ANIMATION);
  build_an_animate(text_layer_get_layer_jf(block_layer), &BLOCK_START, &BLOCK_FINISH, 1000, BLOCK_ANIMATION);
  animation_schedule((Animation*) animations[LOGO_BED_ANIMATION]);
  animation_schedule((Animation*) animations[LOGO_SLEEPER_ANIMATION]);
  animation_schedule((Animation*) animations[LOGO_TEXT_ANIMATION]);
  animation_schedule((Animation*) animations[BLOCK_ANIMATION]);
  text_layer_destroy(version_text);
  text_layer_set_text(block_layer, "");
}

/*
 * Load on window load
 */
static void morpheuz_load(Window *window) {

  window_set_background_color(window, GColorBlack);

  notice_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_16));
  time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_38));

  Layer *window_layer = window_get_root_layer(window);

  powernap_layer = macro_text_layer_create(GRect(5, -3 ,20, 19), window_layer, GColorWhite, GColorBlack, fonts_get_system_font(FONT_KEY_GOTHIC_14), GTextAlignmentCenter);

  macro_bitmap_layer_create(&logo_bed, BED_START, window_layer, RESOURCE_ID_IMAGE_LOGO_BED, true);

  macro_bitmap_layer_create(&logo_sleeper, SLEEPER_START, window_layer, RESOURCE_ID_IMAGE_LOGO_SLEEPER, true);

  macro_bitmap_layer_create(&logo_text, TEXT_START, window_layer, RESOURCE_ID_IMAGE_LOGO_TEXT, true);

  macro_bitmap_layer_create(&logo_head, HEAD_START, window_layer, RESOURCE_ID_IMAGE_LOGO_HEAD, true);

  version_text = macro_text_layer_create(GRect(26, 43, 92, 30), window_layer, GColorWhite, GColorClear, notice_font, GTextAlignmentCenter);
  text_layer_set_text(version_text, VERSION_TXT);

  text_time_layer = macro_text_layer_create(GRect(0, 109, 144, 42), window_layer, GColorWhite, GColorBlack, time_font, GTextAlignmentCenter);

  text_date_layer = macro_text_layer_create(GRect(8, 86, 144-8, 31), window_layer, GColorWhite, GColorBlack, fonts_get_system_font(FONT_KEY_GOTHIC_24), GTextAlignmentCenter);

  smart_alarm_range_layer = macro_text_layer_create(GRect(8, 86, 144-8, 31), window_layer, GColorWhite, GColorBlack, fonts_get_system_font(FONT_KEY_GOTHIC_24), GTextAlignmentCenter);
  layer_set_hidden(text_layer_get_layer_jf(smart_alarm_range_layer), true);

  macro_bitmap_layer_create(&alarm_icon, GRect(144-26-14-10-16-19, ICON_TOPS, 12, 12), window_layer, RESOURCE_ID_ALARM_ICON, false);

  macro_bitmap_layer_create(&weekend, GRect(144-26-14-10-16-19, ICON_TOPS, 12, 12), window_layer, RESOURCE_ID_WEEKEND_ICON, false);

  macro_bitmap_layer_create(&alarm_ring, GRect(144-26-14-10-16-19, ICON_TOPS, 12, 12), window_layer, RESOURCE_ID_ALARM_RING_ICON, false);

  icon_battery = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_ICON);
  icon_battery_charge = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_CHARGE);

  BatteryChargeState initial = battery_state_service_peek();
  battery_level = initial.charge_percent;
  battery_plugged = initial.is_plugged;
  battery_layer = layer_create(GRect(144-26,ICON_TOPS,24,12)); //24*12
  layer_set_update_proc(battery_layer, &battery_layer_update_callback);
  layer_add_child(window_layer, battery_layer);

  macro_bitmap_layer_create(&bluetooth_icon, GRect(144-26-10, ICON_TOPS, 9, 12), window_layer, RESOURCE_ID_BLUETOOTH_ICON, bluetooth_connection_service_peek());

  macro_bitmap_layer_create(&comms_icon, GRect(144-26-14-10, ICON_TOPS, 9, 12), window_layer, RESOURCE_ID_COMMS_ICON, false);

  macro_bitmap_layer_create(&record_icon, GRect(144-26-14-10-16, ICON_TOPS, 10, 12), window_layer, RESOURCE_ID_ICON_RECORD, false);

  macro_bitmap_layer_create(&ignore_icon, GRect(144-26-14-10-16-19-15, ICON_TOPS, 9, 12), window_layer, RESOURCE_ID_IGNORE, false);

  macro_bitmap_layer_create(&activity_icon, GRect(144-26-14-10-16-19-15-15, ICON_TOPS, 9, 12), window_layer, RESOURCE_ID_ACTIVITY_ICON, false);

  progress_layer = layer_create(GRect(17,157,109,9));
  layer_set_update_proc(progress_layer, &progress_layer_update_callback);
  layer_add_child(window_layer, progress_layer);

  block_layer = macro_text_layer_create(BLOCK_START, window_layer, GColorWhite, GColorBlack, notice_font, GTextAlignmentCenter);
  text_layer_set_text(block_layer, COPYRIGHT);

  block_layer2 = macro_text_layer_create(GRect(144-26-14-10-16-19, 0, 144, 18), window_layer, GColorWhite, GColorBlack, notice_font, GTextAlignmentCenter);

  analogue_window_load(window);

  full_inverse_layer = inverter_layer_create(GRect(0, 0, 144, 168));
  layer_add_child(window_layer, inverter_layer_get_layer_jf(full_inverse_layer));
  layer_set_hidden(inverter_layer_get_layer_jf(full_inverse_layer), true);

  read_internal_data();
  read_config_data();

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

  battery_state_service_subscribe(&battery_state_handler);

  bluetooth_connection_service_subscribe(bluetooth_state_handler);

  invert_screen();

  init_morpheuz(window);

  light_enable_interaction();

  app_timer_register(3000, start_animate, NULL);

}

/*
 * Shutdown
 */
static void morpheuz_unload(Window *window) {

  hide_notice_layer(NULL);

  stop_worker();

  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  accel_data_service_unsubscribe();

  save_config_data(NULL);
  save_internal_data();

  inverter_layer_destroy(full_inverse_layer);

  analogue_window_unload();

  layer_destroy(progress_layer);
  layer_destroy(battery_layer);

  text_layer_destroy(text_time_layer);
  text_layer_destroy(text_date_layer);
  text_layer_destroy(smart_alarm_range_layer);
  text_layer_destroy(powernap_layer);

  macro_bitmap_layer_destroy(&logo_bed);
  macro_bitmap_layer_destroy(&logo_sleeper);
  macro_bitmap_layer_destroy(&logo_text);
  macro_bitmap_layer_destroy(&logo_head);
  macro_bitmap_layer_destroy(&alarm_icon);
  macro_bitmap_layer_destroy(&bluetooth_icon);
  macro_bitmap_layer_destroy(&comms_icon);
  macro_bitmap_layer_destroy(&record_icon);
  macro_bitmap_layer_destroy(&alarm_ring);
  macro_bitmap_layer_destroy(&ignore_icon);
  macro_bitmap_layer_destroy(&activity_icon);
  macro_bitmap_layer_destroy(&weekend);

  gbitmap_destroy(icon_battery);
  gbitmap_destroy(icon_battery_charge);

  fonts_unload_custom_font(time_font);
  fonts_unload_custom_font(notice_font);
}

/*
 * Set the weekend indicator on the display
 */
void set_weekend_icon(bool weekend_visible) {
  layer_set_hidden(bitmap_layer_get_layer_jf(weekend.layer), !weekend_visible);
}

/*
 * Set the power nap text for the analogue display
 */
void analogue_powernap_text(char *text) {
  strncpy(powernap_text, text, sizeof(powernap_text));
  text_layer_set_text(powernap_layer, powernap_text);
}

/*
 * Invert screen
 */
void invert_screen() {
  layer_set_hidden(inverter_layer_get_layer_jf(full_inverse_layer), !get_config_data()->invert);
}

/**
 * Hide the bed when making the analogue face visible
 */
void bed_visible(bool value) {
  layer_set_hidden(bitmap_layer_get_layer_jf(logo_bed.layer), !value);
  layer_set_hidden(bitmap_layer_get_layer_jf(logo_head.layer), !value);
}

/*
 * Create main window
 */
static void handle_init() {
  window = window_create();
  window_set_fullscreen(window, true);
  window_set_window_handlers(window, (WindowHandlers ) { .load = morpheuz_load, .unload = morpheuz_unload, });
  window_stack_push(window, true /* Animated */);
}

/**
 * Close main window
 */
void close_morpheuz() {
  window_stack_remove(window, true);
  window_destroy(window);
}

/*
 * Main
 */
int main(void) {
  handle_init();
  app_event_loop();
}
