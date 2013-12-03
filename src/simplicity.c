#include "pebble.h"
#include "morpheuz.h"

Window *window;
TextLayer *text_version_layer;
TextLayer *text_date_layer;
TextLayer *text_time_layer;
Layer *line_layer;
static BitmapLayer *logo_layer;
static GBitmap *logo_bitmap = NULL;
bool g_second = false;
int alert_code = 0;
static uint8_t battery_level;
static bool battery_plugged;

static GBitmap *icon_battery;
static GBitmap *icon_battery_charge;
static Layer *battery_layer;

/*
 * Battery icon callback handler
 */
void battery_layer_update_callback(Layer *layer, GContext *ctx) {

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

void battery_state_handler(BatteryChargeState charge) {
	battery_level = charge.charge_percent;
	battery_plugged = charge.is_plugged;
	layer_mark_dirty(battery_layer);
}

/*
 * Remember an issue we've spotted
 */
void set_alert_code(int new_alert_code) {
	alert_code = alert_code | new_alert_code;
}

/*
 * Draw line
 */
void line_layer_update_callback(Layer *layer, GContext* ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

/*
 * Process clockface
 */
void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  // Need to be static because they're used by the system later.
  static char time_text[] = "00:00";
  static char date_text[] = "Xxxxxxxxx 00";
                             
  char *time_format;
  
  if (alert_code == 0)	
	  strftime(date_text, sizeof(date_text), "%B %e", tick_time);
  else
	  snprintf(date_text, sizeof(date_text), "Issue: %d", alert_code);
  text_layer_set_text(text_date_layer, date_text);

  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  strftime(time_text, sizeof(time_text), time_format, tick_time);

  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (time_text[0] == '0')) {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }

  // Just used to calc size of buffers. Comes back as 23 - going to use 28 to give a little space for movement.
  //Tuplet initial_values[] = {
  //		  TupletInteger(BIGGEST, 0),
  //		  TupletInteger(ALARM, 0),
  //};
	
  //uint32_t test = dict_calc_buffer_size_from_tuplets(initial_values, 2);
  //snprintf(time_text, sizeof(time_text), "%ld", test);

  text_layer_set_text(text_time_layer, time_text);
	
  do_alarm();

}

/*
 * Shutdown
 */
void handle_deinit(void) {
  gbitmap_destroy(icon_battery);
  gbitmap_destroy(icon_battery_charge);
  gbitmap_destroy(logo_bitmap);
  tick_timer_service_unsubscribe();
  deinit_morpheuz();
}

/*
 * Startup
 */
void handle_init(void) {
  static char version[] = VERSION;
	
  window = window_create();
  window_set_fullscreen(window, true);
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack);

  Layer *window_layer = window_get_root_layer(window);
	
  logo_layer = bitmap_layer_create(GRect(0, 5, 144, 80));
  layer_add_child(window_layer, bitmap_layer_get_layer(logo_layer));
  logo_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LOGO);
  bitmap_layer_set_bitmap(logo_layer, logo_bitmap);
	
  text_version_layer = text_layer_create(GRect(48, 0, 48, 16));
  text_layer_set_text_color(text_version_layer, GColorWhite);
  text_layer_set_background_color(text_version_layer, GColorClear);
  text_layer_set_text_alignment(text_version_layer, GTextAlignmentCenter);
  text_layer_set_font(text_version_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(text_version_layer));
  text_layer_set_text(text_version_layer, version);

  text_date_layer = text_layer_create(GRect(8, 85, 144-8, WINDOW_HEIGHT-85));
  text_layer_set_text_color(text_date_layer, GColorWhite);
  text_layer_set_background_color(text_date_layer, GColorClear);
  text_layer_set_font(text_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  layer_add_child(window_layer, text_layer_get_layer(text_date_layer));

  text_time_layer = text_layer_create(GRect(0, 109, 144, WINDOW_HEIGHT-109));
  text_layer_set_text_color(text_time_layer, GColorWhite);
  text_layer_set_text_alignment(text_time_layer, GTextAlignmentCenter);
  text_layer_set_background_color(text_time_layer, GColorClear);
  text_layer_set_font(text_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(text_time_layer));

  GRect line_frame = GRect(8, 114, 144-16, 2);
  line_layer = layer_create(line_frame);
  layer_set_update_proc(line_layer, line_layer_update_callback);
  layer_add_child(window_layer, line_layer);
	
  icon_battery = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_ICON);
  icon_battery_charge = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_CHARGE);

  BatteryChargeState initial = battery_state_service_peek();
  battery_level = initial.charge_percent;
  battery_plugged = initial.is_plugged;
  battery_layer = layer_create(GRect(144-26,2,24,12)); //24*12
  layer_set_update_proc(battery_layer, &battery_layer_update_callback);
  layer_add_child(window_layer, battery_layer);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

  battery_state_service_subscribe(&battery_state_handler);
	
  init_morpheuz();
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
