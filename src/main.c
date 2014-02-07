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

static Window *window;
static TextLayer *text_date_layer;
static TextLayer *text_time_layer;
static Layer *line_layer;
static BitmapLayer *logo_layer;
static GBitmap *logo_bitmap = NULL;
static GBitmap *bluetooth_bitmap = NULL;
static GBitmap *comms_bitmap = NULL;
static bool g_second = false;
static uint8_t battery_level;
static bool battery_plugged;
static InverterLayer *full_inverse_layer;
static BitmapLayer *bluetooth_layer;
static BitmapLayer *comms_layer;
static GBitmap *icon_battery;
static GBitmap *icon_battery_charge;
static Layer *battery_layer;
static bool g_show_special_text = false;
static BitmapLayer *alarm_layer;
static GBitmap *alarm_bitmap;
static char date_text[] = "Xxxxxxxxx 00   ";
//00:00 - 00:00

static Layer *progress_layer;
static uint8_t progress_level;
static GBitmap *image_progress;
static Layer *zzz_layer;
static GBitmap *zzz_icon;
static BitmapLayer *record_layer;
static GBitmap *record_bitmap;

static Window *notice_window;
static BitmapLayer *notice_layer;
static GBitmap *notice_bitmap;
static TextLayer *notice_text;
static bool notice_showing = false;
static GFont notice_font;
static AppTimer *notice_timer;
static char version_text[40];


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
			text_layer_set_text_alignment(text_date_layer, GTextAlignmentLeft);
			layer_set_hidden(bitmap_layer_get_layer(alarm_layer), true);
			show_date();
		}
	} else {
		if (!g_show_special_text) {
			text_layer_set_text_alignment(text_date_layer, GTextAlignmentCenter);
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
		graphics_fill_rect(ctx, GRect(7, 4, (uint8_t)((battery_level / 100.0) * 11.0), 4), 0, GCornerNone);
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
	graphics_draw_bitmap_in_rect(ctx, image_progress, GRect(0, 0, 128, 3));
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, GRect(1, 1, (uint8_t)((progress_level / 100.0) * 127.0), 1), 0, GCornerNone);
}

/*
 * zzz layer
 */
static void zzz_layer_update_callback(Layer *layer, GContext *ctx) {
	graphics_context_set_compositing_mode(ctx, GCompOpAssign);
	graphics_draw_bitmap_in_rect(ctx, zzz_icon, GRect(0, 0, 18, 12));
}

/*
 * Draw line
 */
static void line_layer_update_callback(Layer *layer, GContext* ctx) {
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

/*
 * Process clockface
 */
static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	// Need to be static because they're used by the system later.
	static char time_text[] = "00:00";

	if (!g_show_special_text) {
		show_date();
	}

	clock_copy_time_string	(time_text, sizeof(time_text));
	text_layer_set_text(text_time_layer, time_text);

	self_monitor();

	do_alarm();

	power_nap_countdown();

}

/*
 * Shutdown
 */
static void handle_deinit(void) {
	save_config_data(NULL);
	save_internal_data();
	gbitmap_destroy(icon_battery);
	gbitmap_destroy(icon_battery_charge);
	gbitmap_destroy(logo_bitmap);
	gbitmap_destroy(bluetooth_bitmap);
	gbitmap_destroy(comms_bitmap);
	gbitmap_destroy(alarm_bitmap);
	gbitmap_destroy(image_progress);
	gbitmap_destroy(zzz_icon);
	gbitmap_destroy(record_bitmap);
	gbitmap_destroy(notice_bitmap);
	fonts_unload_custom_font(notice_font);
	tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
	bluetooth_connection_service_unsubscribe();
	deinit_morpheuz();
}

/*
 * Bluetooth connection status
 */
static void bluetooth_state_handler(bool connected) {
	layer_set_hidden(bitmap_layer_get_layer(bluetooth_layer), !connected);
}

/*
 * Comms connection status
 */
void show_comms_state(bool connected) {
	layer_set_hidden(bitmap_layer_get_layer(comms_layer), !connected);
}

/*
 * Progress indicator position
 */
void set_progress(uint8_t progress_percent) {
	progress_level = progress_percent;
	layer_mark_dirty(progress_layer);
}

/*
 * Toggle zzz
 */
void toggle_zzz() {
	layer_set_hidden(zzz_layer, !layer_get_hidden(zzz_layer));
}

/*
 * Show record
 */
void show_record(bool recording) {
	layer_set_hidden(bitmap_layer_get_layer(record_layer), !recording);
}

/*
 * Remove the notice window
 */
static void hide_notice_layer(void *data) {
	if (notice_showing) {
		window_stack_remove(notice_window, true);
		window_destroy(notice_window);
		notice_showing = false;
	}
}

/*
 * Show the notice window
 */
void show_notice(char *message) {
	if (notice_showing) {
		text_layer_set_text(notice_text, message);
		app_timer_cancel(notice_timer);
		notice_timer = app_timer_register(NOTICE_DISPLAY_MS, hide_notice_layer, NULL);
		return;
	}

	notice_showing = true;
	notice_window = window_create();
	window_set_fullscreen(notice_window, true);
	window_stack_push(notice_window, true /* Animated */);
	window_set_background_color(notice_window, GColorBlack);

	Layer *window_layer = window_get_root_layer(notice_window);

	notice_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
	layer_add_child(window_layer, bitmap_layer_get_layer(notice_layer));

	bitmap_layer_set_bitmap(notice_layer, notice_bitmap);

	notice_text = text_layer_create(GRect(17, 61, 112, 65));
	text_layer_set_text_color(notice_text, GColorBlack);
	text_layer_set_background_color(notice_text, GColorClear);
	text_layer_set_text_alignment(notice_text, GTextAlignmentCenter);
	text_layer_set_font(notice_text, notice_font);
	layer_add_child(window_layer, text_layer_get_layer(notice_text));
	text_layer_set_text(notice_text, message);

	window_set_click_config_provider(notice_window, (ClickConfigProvider) click_config_provider);

	notice_timer = app_timer_register(NOTICE_DISPLAY_MS, hide_notice_layer, NULL);
}

/*
 * Startup
 */
static void handle_init(void) {

	window = window_create();
	window_set_fullscreen(window, true);

	window_set_background_color(window, GColorBlack);

	Layer *window_layer = window_get_root_layer(window);

	logo_layer = bitmap_layer_create(GRect(0, 10, 144, 80));
	layer_add_child(window_layer, bitmap_layer_get_layer(logo_layer));
	logo_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LOGO);
	bitmap_layer_set_bitmap(logo_layer, logo_bitmap);

	text_date_layer = text_layer_create(GRect(8, 85, 144-8, WINDOW_HEIGHT-85));
	text_layer_set_text_color(text_date_layer, GColorWhite);
	text_layer_set_background_color(text_date_layer, GColorBlack);
	text_layer_set_font(text_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
	layer_add_child(window_layer, text_layer_get_layer(text_date_layer));

	text_time_layer = text_layer_create(GRect(0, 109, 144, WINDOW_HEIGHT-109));
	text_layer_set_text_color(text_time_layer, GColorWhite);
	text_layer_set_text_alignment(text_time_layer, GTextAlignmentCenter);
	text_layer_set_background_color(text_time_layer, GColorBlack);
	text_layer_set_font(text_time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_38)));
	layer_add_child(window_layer, text_layer_get_layer(text_time_layer));

	alarm_layer = bitmap_layer_create(GRect(11, 98, 8, 12));
	layer_add_child(window_layer, bitmap_layer_get_layer(alarm_layer));
	alarm_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ALARM_ICON);
	bitmap_layer_set_bitmap(alarm_layer, alarm_bitmap);
	layer_set_hidden(bitmap_layer_get_layer(alarm_layer), true);

	GRect line_frame = GRect(8, 114, 144-16, 2);
	line_layer = layer_create(line_frame);
	layer_set_update_proc(line_layer, line_layer_update_callback);
	layer_add_child(window_layer, line_layer);

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

	image_progress = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PROGRESS);
	progress_level = 0;
	progress_layer = layer_create(GRect(8,153,128,3));
	layer_set_update_proc(progress_layer, &progress_layer_update_callback);
	layer_add_child(window_layer, progress_layer);

	zzz_icon = gbitmap_create_with_resource(RESOURCE_ID_ICON_ZZZ);
	zzz_layer = layer_create(GRect(41,11,18,12));
	layer_set_update_proc(zzz_layer, &zzz_layer_update_callback);
	layer_add_child(window_layer, zzz_layer);

	full_inverse_layer = inverter_layer_create(GRect(0, 0, 144, 168));
	layer_add_child(window_layer, inverter_layer_get_layer(full_inverse_layer));
	layer_set_hidden(inverter_layer_get_layer(full_inverse_layer), true);

	notice_bitmap = gbitmap_create_with_resource(RESOURCE_ID_NOTICE_BG);
	notice_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_16));

	window_stack_push(window, true /* Animated */);

	read_internal_data();
	read_config_data();

	set_progress_based_on_persist();

	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

	battery_state_service_subscribe(&battery_state_handler);

	bluetooth_connection_service_subscribe(bluetooth_state_handler);

	invert_screen();

	init_morpheuz(window);

	// Show the welcome and version
	snprintf(version_text, sizeof(version_text), NOTICE_WELCOME, VERSION_TXT);
	show_notice(version_text);

}



/*
 * Invert screen
 */
void invert_screen() {
	layer_set_hidden(inverter_layer_get_layer(full_inverse_layer), !get_config_data()->invert);
}

/*
 * Shorten the tick interval whilst the alarm is going off
 */
void reset_tick_service(bool second) {
	if (second == g_second)
		return;
	g_second = second;
	tick_timer_service_unsubscribe();
	if (second)
		tick_timer_service_subscribe(SECOND_UNIT, handle_minute_tick);
	else
		tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

/*
 * Main
 */
int main(void) {
	handle_init();

	app_event_loop();

	handle_deinit();
}
