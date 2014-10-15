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

static Window *window;

static Layer *progress_layer;
static Layer *battery_layer;

static TextLayer *text_date_layer;
static TextLayer *text_time_layer;
static TextLayer *version_text;
static TextLayer *block_layer;
static TextLayer *block_layer2;

static BitmapLayer *logo_bed_layer;
static BitmapLayer *logo_sleeper_layer;
static BitmapLayer *logo_head_layer;
static BitmapLayer *logo_text_layer;
static BitmapLayer *bluetooth_layer;
static BitmapLayer *activity_layer;
static BitmapLayer *comms_layer;
static BitmapLayer *ignore_layer;
static BitmapLayer *record_layer;
static BitmapLayer *alarm_icon_layer;
static BitmapLayer *alarm_layer;

static InverterLayer *full_inverse_layer;

static GBitmap *alarm_bitmap;
static GBitmap *alarm_ring_bitmap;
static GBitmap *icon_battery;
static GBitmap *icon_battery_charge;
static GBitmap *image_progress;
static GBitmap *image_progress_full;
static GBitmap *record_bitmap;
static GBitmap *logo_bed_bitmap;
static GBitmap *logo_sleeper_bitmap;
static GBitmap *logo_head_bitmap;
static GBitmap *logo_text_bitmap;
static GBitmap *bluetooth_bitmap;
static GBitmap *activity_bitmap;
static GBitmap *comms_bitmap;
static GBitmap *ignore_bitmap;

extern GFont notice_font;
static GFont time_font;

static struct PropertyAnimation *logo_bed_animation;
static struct PropertyAnimation *logo_sleeper_animation;
static struct PropertyAnimation *logo_text_animation;
static struct PropertyAnimation *block_animation;
static struct PropertyAnimation *logo_head_animation;

static uint8_t battery_level;
static bool battery_plugged;
static bool g_show_special_text = false;
static char date_text[16];
static uint16_t progress_level;
static bool first_time = true;
static uint8_t animation_count = 0;

/*
 * Boot up background process
 */
static void start_worker() {
  AppWorkerResult result = app_worker_launch();
  if (result == APP_WORKER_RESULT_SUCCESS ||
      result == APP_WORKER_RESULT_ALREADY_RUNNING) {
    layer_set_hidden(bitmap_layer_get_layer(activity_layer), false);
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
  layer_set_hidden(bitmap_layer_get_layer(activity_layer), !app_worker_is_running());
}

/*
 * Show the date
 */
static void show_date() {
	time_t now = time(NULL);
	struct tm *time = localtime(&now);
	strftime(date_text, sizeof(date_text), "%B %e", time);
	text_layer_set_text(text_date_layer, date_text);
}

/*
 * Set the smart alarm status details
 */
void set_smart_status_on_screen(bool show_special_text, char *special_text) {
	if (!show_special_text) {
		if (g_show_special_text) {
			layer_set_hidden(bitmap_layer_get_layer(alarm_layer), true);
			show_date();
		}
	} else {
		if (!g_show_special_text) {
			layer_set_hidden(bitmap_layer_get_layer(alarm_layer), false);
		}
		strncpy(date_text, special_text, sizeof(date_text));
		text_layer_set_text(text_date_layer, date_text);
	}
	g_show_special_text = show_special_text;
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
	graphics_context_set_compositing_mode(ctx, GCompOpAssign);
	graphics_draw_bitmap_in_rect(ctx, image_progress, GRect(0, 0, 131, 5));
	graphics_draw_bitmap_in_rect(ctx, image_progress_full, GRect(0, 0, progress_level * 131 / LIMIT, 5));
}

/*
 * Process clockface
 */
static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {

	if (!g_show_special_text) {
		show_date();
	}

	set_text_layer_to_time(text_time_layer);

	self_monitor();

	if (!first_time) {
	  every_minute_processing(tick_time->tm_min);
	  show_worker();
	} else {
		first_time = false;
	}

	power_nap_countdown();
}

/*
 * Bluetooth connection status
 */
static void bluetooth_state_handler(bool connected) {
	layer_set_hidden(bitmap_layer_get_layer(bluetooth_layer), !connected);
	if (!connected)
		show_comms_state(false); // because we only set comms state on NAK/ACK it can be at odds with BT state - do this else that is confusing
}


/*
 * Comms connection status
 */
void show_comms_state(bool connected) {
	layer_set_hidden(bitmap_layer_get_layer(comms_layer), !connected);
}

/*
 * Ignore status
 */
void show_ignore_state(bool ignore) {
  layer_set_hidden(bitmap_layer_get_layer(ignore_layer), !ignore);
}

/*
 * Alarm status icon
 */
void set_alarm_icon(bool show_icon) {
	layer_set_hidden(bitmap_layer_get_layer(alarm_icon_layer), !show_icon);
}

/*
 * Progress indicator position (1-54)
 */
void set_progress(uint8_t progress_level_in) {
	progress_level = progress_level_in;
	layer_mark_dirty(progress_layer);
}

/*
 * Show record
 */
void show_record(bool recording) {
	layer_set_hidden(bitmap_layer_get_layer(record_layer), !recording);
}

/*
 * End of initial animation sequence - occurs 4 then 5 times
 */
static void animation_stopped(Animation *animation, bool finished, void *data) {
  animation_count++;
  if (animation_count == 4) {
    light_enable_interaction();
    animation_unschedule((Animation*) logo_bed_animation);
    animation_unschedule((Animation*) logo_sleeper_animation);
    animation_unschedule((Animation*) logo_text_animation);
    animation_unschedule((Animation*) block_animation);
    animation_destroy((Animation*) logo_bed_animation);
    animation_destroy((Animation*) logo_sleeper_animation);
    animation_destroy((Animation*) logo_text_animation);
    animation_destroy((Animation*) block_animation);
    text_layer_destroy(block_layer2);
    animation_schedule((Animation*) logo_head_animation);
  } else if (animation_count == 5) {
    animation_unschedule((Animation*) logo_head_animation);
    animation_destroy((Animation*) logo_head_animation);
    start_worker();
  }
}

/*
 * Start title animation sequence
 */
static void start_animate(void *data) {
  light_enable_interaction();

  GRect start = BED_START;
  GRect finish = BED_FINISH;
  logo_bed_animation = property_animation_create_layer_frame(bitmap_layer_get_layer(logo_bed_layer), &start, &finish);
  animation_set_duration((Animation*) logo_bed_animation, 1000);
  animation_set_handlers((Animation*) logo_bed_animation, (AnimationHandlers ) { .stopped = (AnimationStoppedHandler) animation_stopped, }, NULL /* callback data */);

  start = SLEEPER_START;
  finish = SLEEPER_FINISH;
  logo_sleeper_animation = property_animation_create_layer_frame(bitmap_layer_get_layer(logo_sleeper_layer), &start, &finish);
  animation_set_duration((Animation*) logo_sleeper_animation, 1000);
  animation_set_handlers((Animation*) logo_sleeper_animation, (AnimationHandlers ) { .stopped = (AnimationStoppedHandler) animation_stopped, }, NULL /* callback data */);

  start = HEAD_START;
  finish = HEAD_FINISH;
  logo_head_animation = property_animation_create_layer_frame(bitmap_layer_get_layer(logo_head_layer), &start, &finish);
  animation_set_duration((Animation*) logo_head_animation, 500);
  animation_set_handlers((Animation*) logo_head_animation, (AnimationHandlers ) { .stopped = (AnimationStoppedHandler) animation_stopped, }, NULL /* callback data */);

  start = TEXT_START;
  finish = TEXT_FINISH;
  logo_text_animation = property_animation_create_layer_frame(bitmap_layer_get_layer(logo_text_layer), &start, &finish);
  animation_set_duration((Animation*) logo_text_animation, 1000);
  animation_set_handlers((Animation*) logo_text_animation, (AnimationHandlers ) { .stopped = (AnimationStoppedHandler) animation_stopped, }, NULL /* callback data */);

  start = BLOCK_START;
  finish = BLOCK_FINISH;
  block_animation = property_animation_create_layer_frame(text_layer_get_layer(block_layer), &start, &finish);
  animation_set_duration((Animation*) block_animation, 1000);
  animation_set_handlers((Animation*) block_animation, (AnimationHandlers ) { .stopped = (AnimationStoppedHandler) animation_stopped, }, NULL /* callback data */);

  animation_schedule((Animation*) logo_bed_animation);
  animation_schedule((Animation*) logo_sleeper_animation);
  animation_schedule((Animation*) logo_text_animation);
  animation_schedule((Animation*) block_animation);
  text_layer_destroy(version_text);
  text_layer_set_text(block_layer, "");
}

/*
 * Startup
 */
static void handle_init(void) {

	window = window_create();
	window_set_fullscreen(window, true);

	window_set_background_color(window, GColorBlack);

	notice_init();

	Layer *window_layer = window_get_root_layer(window);

	logo_bed_layer = bitmap_layer_create(BED_START);
	layer_add_child(window_layer, bitmap_layer_get_layer(logo_bed_layer));
	logo_bed_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LOGO_BED);
	bitmap_layer_set_bitmap(logo_bed_layer, logo_bed_bitmap);

  logo_sleeper_layer = bitmap_layer_create(SLEEPER_START);
  layer_add_child(window_layer, bitmap_layer_get_layer(logo_sleeper_layer));
  logo_sleeper_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LOGO_SLEEPER);
  bitmap_layer_set_bitmap(logo_sleeper_layer, logo_sleeper_bitmap);

  logo_text_layer = bitmap_layer_create(TEXT_START);
  layer_add_child(window_layer, bitmap_layer_get_layer(logo_text_layer));
  logo_text_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LOGO_TEXT);
  bitmap_layer_set_bitmap(logo_text_layer, logo_text_bitmap);

  logo_head_layer = bitmap_layer_create(HEAD_START);
  layer_add_child(window_layer, bitmap_layer_get_layer(logo_head_layer));
  logo_head_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LOGO_HEAD);
  bitmap_layer_set_bitmap(logo_head_layer, logo_head_bitmap);

  version_text = macro_text_layer_create(GRect(26, 43, 92, 30),window_layer,GColorWhite,GColorClear, notice_font, GTextAlignmentCenter);
  text_layer_set_text(version_text, VERSION_TXT);

	time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_38));
	text_time_layer = macro_text_layer_create(GRect(0, 109, 144, 42),window_layer,GColorWhite,GColorBlack, time_font, GTextAlignmentCenter);

	text_date_layer = macro_text_layer_create(GRect(8, 85, 144-8, 31),window_layer,GColorWhite,GColorBlack, fonts_get_system_font(FONT_KEY_GOTHIC_24), GTextAlignmentCenter);

	alarm_layer = bitmap_layer_create(GRect(11, 96, 12, 12));
	layer_add_child(window_layer, bitmap_layer_get_layer(alarm_layer));
	alarm_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ALARM_ICON);
	bitmap_layer_set_bitmap(alarm_layer, alarm_bitmap);
	layer_set_hidden(bitmap_layer_get_layer(alarm_layer), true);

	icon_battery = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_ICON);
	icon_battery_charge = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_CHARGE);

	BatteryChargeState initial = battery_state_service_peek();
	battery_level = initial.charge_percent;
	battery_plugged = initial.is_plugged;
	battery_layer = layer_create(GRect(144-26,4,24,12)); //24*12
	layer_set_update_proc(battery_layer, &battery_layer_update_callback);
	layer_add_child(window_layer, battery_layer);

	bluetooth_layer = bitmap_layer_create(GRect(144-26-10, 4, 9, 12));
	layer_add_child(window_layer, bitmap_layer_get_layer(bluetooth_layer));
	bluetooth_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BLUETOOTH_ICON);
	bitmap_layer_set_bitmap(bluetooth_layer, bluetooth_bitmap);
	layer_set_hidden(bitmap_layer_get_layer(bluetooth_layer), !bluetooth_connection_service_peek());

	comms_layer = bitmap_layer_create(GRect(144-26-14-10, 4, 9, 12));
	layer_add_child(window_layer, bitmap_layer_get_layer(comms_layer));
	comms_bitmap = gbitmap_create_with_resource(RESOURCE_ID_COMMS_ICON);
	bitmap_layer_set_bitmap(comms_layer, comms_bitmap);
	layer_set_hidden(bitmap_layer_get_layer(comms_layer), true);

	record_layer = bitmap_layer_create(GRect(144-26-14-10-16, 4, 10, 12));
	layer_add_child(window_layer, bitmap_layer_get_layer(record_layer));
	record_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_RECORD);
	bitmap_layer_set_bitmap(record_layer, record_bitmap);
	layer_set_hidden(bitmap_layer_get_layer(record_layer), true);

	alarm_ring_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ALARM_RING_ICON);
	alarm_icon_layer = bitmap_layer_create(GRect(144-26-14-10-16-19, 4, 12, 12));
	layer_add_child(window_layer, bitmap_layer_get_layer(alarm_icon_layer));
	bitmap_layer_set_bitmap(alarm_icon_layer, alarm_ring_bitmap);
	layer_set_hidden(bitmap_layer_get_layer(alarm_icon_layer), true);

  ignore_layer = bitmap_layer_create(GRect(144-26-14-10-16-19-15, 4, 9, 12));
  layer_add_child(window_layer, bitmap_layer_get_layer(ignore_layer));
  ignore_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IGNORE);
  bitmap_layer_set_bitmap(ignore_layer, ignore_bitmap);
  layer_set_hidden(bitmap_layer_get_layer(ignore_layer), true);

  activity_layer = bitmap_layer_create(GRect(144-26-14-10-16-19-15-15, 4, 9, 12));
  layer_add_child(window_layer, bitmap_layer_get_layer(activity_layer));
  activity_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ACTIVITY_ICON);
  bitmap_layer_set_bitmap(activity_layer, activity_bitmap);
  layer_set_hidden(bitmap_layer_get_layer(activity_layer), true);

	image_progress = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PROGRESS);
	image_progress_full = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PROGRESS_FULL);
	progress_level = 0;
	progress_layer = layer_create(GRect(6,161,131,5));
	layer_set_update_proc(progress_layer, &progress_layer_update_callback);
	layer_add_child(window_layer, progress_layer);

	block_layer = macro_text_layer_create(BLOCK_START,window_layer,GColorWhite,GColorBlack, notice_font, GTextAlignmentCenter);
  text_layer_set_text(block_layer, COPYRIGHT);

  block_layer2 = macro_text_layer_create(GRect(144-26-14-10-16-19, 0, 144, 18),window_layer,GColorWhite,GColorBlack, notice_font, GTextAlignmentCenter);

	full_inverse_layer = inverter_layer_create(GRect(0, 0, 144, 168));
	layer_add_child(window_layer, inverter_layer_get_layer(full_inverse_layer));
	layer_set_hidden(inverter_layer_get_layer(full_inverse_layer), true);

	window_stack_push(window, true /* Animated */);

	read_internal_data();
	read_config_data();

	set_progress_based_on_persist();

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
static void handle_deinit(void) {
  stop_worker();

  save_config_data(NULL);
	save_internal_data();
  notice_deinit();

  window_stack_remove(window, true);

  inverter_layer_destroy(full_inverse_layer);

  layer_destroy(progress_layer);
  layer_destroy(battery_layer);

  text_layer_destroy(text_time_layer);
  text_layer_destroy(text_date_layer);

  bitmap_layer_destroy(logo_bed_layer);
  bitmap_layer_destroy(logo_sleeper_layer);
  bitmap_layer_destroy(logo_head_layer);
  bitmap_layer_destroy(logo_text_layer);
  bitmap_layer_destroy(comms_layer);
  bitmap_layer_destroy(ignore_layer);
  bitmap_layer_destroy(bluetooth_layer);
  bitmap_layer_destroy(activity_layer);
  bitmap_layer_destroy(alarm_icon_layer);
  bitmap_layer_destroy(record_layer);
  bitmap_layer_destroy(alarm_layer);

  gbitmap_destroy(alarm_bitmap);
  gbitmap_destroy(alarm_ring_bitmap);
  gbitmap_destroy(icon_battery);
  gbitmap_destroy(icon_battery_charge);
  gbitmap_destroy(image_progress);
  gbitmap_destroy(image_progress_full);
  gbitmap_destroy(record_bitmap);
  gbitmap_destroy(logo_bed_bitmap);
  gbitmap_destroy(logo_sleeper_bitmap);
  gbitmap_destroy(logo_head_bitmap);
  gbitmap_destroy(logo_text_bitmap);
	gbitmap_destroy(bluetooth_bitmap);
  gbitmap_destroy(activity_bitmap);
	gbitmap_destroy(comms_bitmap);
  gbitmap_destroy(ignore_bitmap);

  fonts_unload_custom_font(time_font);

	tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
	bluetooth_connection_service_unsubscribe();

	deinit_morpheuz();

	window_destroy(window);
}

/*
 * Invert screen
 */
void invert_screen() {
	layer_set_hidden(inverter_layer_get_layer(full_inverse_layer), !get_config_data()->invert);
}

/*
 * Main
 */
int main(void) {
	handle_init();

	app_event_loop();

	handle_deinit();
}
