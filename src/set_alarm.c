/*
 * Morpheuz Sleep Monitor
 *
 * Copyright (c) 2013-2014 James Fowler
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
#include "language.h"
#include "morpheuz.h"

// Field definitions
#define F_FROM_HOUR 1
#define F_FROM_MINUTE 2
#define F_TO_HOUR 3
#define F_TO_MINUTE 4
#define F_SMART_ALARM 0
#define F_DONE 5
#define MAX_SETTINGS_FIELDS 6

// Position and size info
#define SETTING_TITLE_LEFT 5
#define SETTING_FROM_TOP 50
#define SETTING_TO_TOP 100
#define SETTING_BIG_FONT_HEIGHT 30
#define SETTING_SMALL_FONT_HEIGHT 20
#define SETTING_TIME_WIDTH 25

static Window *setting_window;
static GBitmap *up_button_res;
static GBitmap *next_button_res;
static GBitmap *down_button_res;
static GBitmap *tick_button_res;
static GFont small_font;
static GFont large_font;
static ActionBarLayer *button_layer;
static TextLayer *title_layer;
static TextLayer *from_layer;
static TextLayer *from_colon_layer;
static TextLayer *to_layer;
static TextLayer *to_color_layer;

static TextLayer *fields[MAX_SETTINGS_FIELDS];
static int values[MAX_SETTINGS_FIELDS];
static char value_text[5][MAX_SETTINGS_FIELDS];

static int8_t current_field;

/*
 * Highlight or unhighlight a field
 */
static void highlight_field(int8_t id, bool hilight) {
  text_layer_set_background_color(fields[id], hilight ? GColorBlack : GColorWhite);
  text_layer_set_text_color(fields[id], hilight ? GColorWhite : GColorBlack);
  if (hilight) {
    action_bar_layer_set_icon(button_layer, BUTTON_ID_UP, id == F_DONE ? tick_button_res : up_button_res);
    action_bar_layer_set_icon(button_layer, BUTTON_ID_DOWN, id == F_DONE ? tick_button_res : down_button_res);
  }
}

/*
 * Write a time value into a field
 */
static void write_time_value_to_field(int8_t id) {
  snprintf(value_text[id], sizeof(value_text[id]), "%02d", values[id]);
  text_layer_set_text(fields[id], value_text[id]);
}

/*
 * Write values from memory into fields
 */
static void write_values_to_fields() {
  write_time_value_to_field(F_FROM_HOUR);
  write_time_value_to_field(F_FROM_MINUTE);
  write_time_value_to_field(F_TO_HOUR);
  write_time_value_to_field(F_TO_MINUTE);
  text_layer_set_text(fields[F_SMART_ALARM], values[F_SMART_ALARM] == 1 ? ON : OFF);
  text_layer_set_text(fields[F_DONE], values[F_DONE] == 1 ? DONE : BAD);
}

/*
 * Select button moves to next field - highlight and move
 */
static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  highlight_field(current_field, false);
  current_field++;
  if (current_field > F_DONE)
    current_field = F_SMART_ALARM;
  highlight_field(current_field, true);
}

/*
 * Change a setting value within constraints
 */
static void alter_time(int8_t id, int8_t delta, int8_t top, int8_t bottom) {
  values[id] += delta;
  if (values[id] < bottom)
    values[id] = top;
  else if (values[id] > top)
    values[id] = bottom;
}

/*
 * Drop values back into the rest of the system
 */
static void return_values() {
  get_config_data()->smart = values[F_SMART_ALARM];
  get_config_data()->fromhr = values[F_FROM_HOUR];
  get_config_data()->frommin = values[F_FROM_MINUTE];
  get_config_data()->tohr = values[F_TO_HOUR];
  get_config_data()->tomin = values[F_TO_MINUTE];
  get_config_data()->from = to_mins(get_config_data()->fromhr, get_config_data()->frommin);
  get_config_data()->to = to_mins(get_config_data()->tohr, get_config_data()->tomin);
  resend_all_data(true); // Force resend - we've fiddled with the times
  trigger_config_save();
  set_smart_status();
}

/*
 * Close the settings page
 */
static void hide_set_alarm(void) {
  return_values();
  window_stack_remove(setting_window, true);
}

/*
 * Handle an up or down button press and validate results
 */
static void up_down_handler(int8_t delta) {
  switch (current_field) {
    case F_FROM_HOUR:
    case F_TO_HOUR:
      alter_time(current_field, delta, 23, 0);
      break;
    case F_FROM_MINUTE:
    case F_TO_MINUTE:
      alter_time(current_field, 5 * delta, 55, 0);
      break;
    case F_SMART_ALARM:
      alter_time(current_field, 1, 1, 0);
      break;
    case F_DONE:
      if (values[F_DONE] == 1)
        hide_set_alarm();
      else
        vibes_short_pulse();
      break;
  }
  uint32_t from = to_mins(values[F_FROM_HOUR], values[F_FROM_MINUTE]);
  uint32_t to = to_mins(values[F_TO_HOUR], values[F_TO_MINUTE]);
  values[F_DONE] = (from <= to);
  write_values_to_fields();
}

/*
 * Single click handler on up button
 */
static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  up_down_handler(1);
}

/*
 * Single click handler on down button
 */
static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  up_down_handler(-1);
}

/*
 * Button click handlers
 */
static void setting_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);
}

/*
 * Set the fields to the values from the rest of the system
 */
static void set_values() {
  values[F_SMART_ALARM] = get_config_data()->smart;
  values[F_FROM_HOUR] = get_config_data()->fromhr;
  values[F_FROM_MINUTE] = get_config_data()->frommin;
  values[F_TO_HOUR] = get_config_data()->tohr;
  values[F_TO_MINUTE] = get_config_data()->tomin;
  values[F_DONE] = 1;
}

/*
 * Build the settings window
 */
static void create_settings_window(void) {
  setting_window = window_create();
  Layer *window_layer = window_get_root_layer(setting_window);

  // Get the resources we need
  up_button_res = gbitmap_create_with_resource(RESOURCE_ID_PICK_UP);
  next_button_res = gbitmap_create_with_resource(RESOURCE_ID_PICK_NEXT);
  down_button_res = gbitmap_create_with_resource(RESOURCE_ID_PICK_DOWN);
  tick_button_res = gbitmap_create_with_resource(RESOURCE_ID_PICK_TICK);
  small_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  large_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);

  // button_layer
  button_layer = action_bar_layer_create();
  action_bar_layer_add_to_window(button_layer, setting_window);
  action_bar_layer_set_background_color(button_layer, GColorBlack);
  action_bar_layer_set_icon(button_layer, BUTTON_ID_UP, up_button_res);
  action_bar_layer_set_icon(button_layer, BUTTON_ID_SELECT, next_button_res);
  action_bar_layer_set_icon(button_layer, BUTTON_ID_DOWN, down_button_res);
  layer_add_child(window_layer, (Layer *) button_layer);
  action_bar_layer_set_click_config_provider(button_layer, setting_click_config_provider);

  // title_layer
  title_layer = macro_text_layer_create(GRect(SETTING_TITLE_LEFT, 6, 90, SETTING_SMALL_FONT_HEIGHT), window_layer, GColorBlack, GColorWhite, small_font, GTextAlignmentLeft);
  text_layer_set_text(title_layer, SMART_ALARM);

  // from_layer
  from_layer = macro_text_layer_create(GRect(SETTING_TITLE_LEFT, 30, 55, SETTING_SMALL_FONT_HEIGHT), window_layer, GColorBlack, GColorWhite, small_font, GTextAlignmentLeft);
  text_layer_set_text(from_layer, EARLIEST);

  // fields[FROM_HOUR]
  fields[F_FROM_HOUR] = macro_text_layer_create(GRect(20, SETTING_FROM_TOP, SETTING_TIME_WIDTH, SETTING_BIG_FONT_HEIGHT), window_layer, GColorBlack, GColorWhite, large_font, GTextAlignmentCenter);

  // from_colon_layer
  from_colon_layer = macro_text_layer_create(GRect(43, SETTING_FROM_TOP, 13, SETTING_BIG_FONT_HEIGHT), window_layer, GColorBlack, GColorWhite, large_font, GTextAlignmentCenter);
  text_layer_set_text(from_colon_layer, COLON);

  // fields[FROM_MINUTE]
  fields[F_FROM_MINUTE] = macro_text_layer_create(GRect(52, SETTING_FROM_TOP, SETTING_TIME_WIDTH, SETTING_BIG_FONT_HEIGHT), window_layer, GColorBlack, GColorWhite, large_font, GTextAlignmentCenter);

  // to_layer
  to_layer = macro_text_layer_create(GRect(SETTING_TITLE_LEFT, 81, 48, SETTING_SMALL_FONT_HEIGHT), window_layer, GColorBlack, GColorWhite, small_font, GTextAlignmentLeft);
  text_layer_set_text(to_layer, LATEST);

  // fields[TO_HOUR]
  fields[F_TO_HOUR] = macro_text_layer_create(GRect(20, SETTING_TO_TOP, SETTING_TIME_WIDTH, SETTING_BIG_FONT_HEIGHT), window_layer, GColorBlack, GColorWhite, large_font, GTextAlignmentCenter);

  // to_color_layer
  to_color_layer = macro_text_layer_create(GRect(43, SETTING_TO_TOP, 13, SETTING_BIG_FONT_HEIGHT), window_layer, GColorBlack, GColorWhite, large_font, GTextAlignmentCenter);
  text_layer_set_text(to_color_layer, COLON);

  // fields[TO_MINUTE]
  fields[F_TO_MINUTE] = macro_text_layer_create(GRect(52, SETTING_TO_TOP, SETTING_TIME_WIDTH, SETTING_BIG_FONT_HEIGHT), window_layer, GColorBlack, GColorWhite, large_font, GTextAlignmentCenter);

  // fields[SMART_ALARM]
  fields[F_SMART_ALARM] = macro_text_layer_create(GRect(94, 1, 25, SETTING_BIG_FONT_HEIGHT), window_layer, GColorBlack, GColorWhite, large_font, GTextAlignmentCenter);

  // fields[DONE]
  fields[F_DONE] = macro_text_layer_create(GRect(77, 118, 45, SETTING_BIG_FONT_HEIGHT), window_layer, GColorBlack, GColorWhite, large_font, GTextAlignmentCenter);

  current_field = F_SMART_ALARM;

  set_values();
  write_values_to_fields();
  highlight_field(current_field, true);
}

/*
 * Throw away the resources we gathered
 */
static void dispose_resources(void) {
  window_destroy(setting_window);
  action_bar_layer_destroy(button_layer);
  text_layer_destroy(title_layer);
  text_layer_destroy(from_layer);
  text_layer_destroy(fields[F_FROM_HOUR]);
  text_layer_destroy(fields[F_FROM_MINUTE]);
  text_layer_destroy(from_colon_layer);
  text_layer_destroy(to_layer);
  text_layer_destroy(fields[F_TO_HOUR]);
  text_layer_destroy(to_color_layer);
  text_layer_destroy(fields[F_TO_MINUTE]);
  text_layer_destroy(fields[F_SMART_ALARM]);
  text_layer_destroy(fields[F_DONE]);
  gbitmap_destroy(up_button_res);
  gbitmap_destroy(next_button_res);
  gbitmap_destroy(tick_button_res);
  gbitmap_destroy(down_button_res);
}

/*
 * Unload window
 */
static void handle_window_unload(Window* window) {
  dispose_resources();
}

/*
 * Display window
 */
void show_set_alarm() {
  create_settings_window();
  window_set_window_handlers(setting_window, (WindowHandlers ) { .unload = handle_window_unload, });
  window_stack_push(setting_window, true);
}

