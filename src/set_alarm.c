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
#include "language.h"
#include "morpheuz.h"

// Field definitions
#define F_FROM_HOUR 0
#define F_FROM_MINUTE 1
#define F_FROM_AMPM 2
#define F_TO_HOUR 3
#define F_TO_MINUTE 4
#define F_TO_AMPM 5
#define MAX_SETTINGS_FIELDS 6
 
// Position and size stuff
#define SA_ROW_1_Y 8
#define SA_ROW_2_Y 42
#define SA_ROW_3_Y 77
#define SA_ROW_4_Y 111
  
#define SA_HEIGHT 34
#define SA_WIDTH 40

#define SA_ROW_1_X (0)
#define SA_ROW_3_X (0)
#define SA_ROW_1_WIDTH (width)
#define SA_ROW_3_WIDTH (width)
  
#define SA_HOUR_LEFT_12 (8 - 72 + centre)
#define SA_HOUR_LEFT_24 (30 - 72 + centre)
#define SA_AMPM_LEFT (96 - 72 + centre)
#define SA_MIN_LEFT_12 (52 - 72 + centre)
#define SA_MIN_LEFT_24 (74 - 72 + centre)

static Window *setting_window;
static GFont large_font;
static GFont small_font;
static TextLayer *from_layer;
static TextLayer *to_layer;

static TextLayer *fields[MAX_SETTINGS_FIELDS];
static int values[MAX_SETTINGS_FIELDS];
static char value_text[5][MAX_SETTINGS_FIELDS];

static int8_t current_field;

static bool is24hr;

/*
 * Highlight or unhighlight a field
 */
static void highlight_field(int8_t id, bool hilight) {
  text_layer_set_background_color(fields[id], hilight ? HIGHLIGHT_BG_COLOR : NON_HIGHLIGHT_BG_COLOR);
  text_layer_set_text_color(fields[id], hilight ? HIGHLIGHT_FG_COLOR : NON_HIGHLIGHT_FG_COLOR);
}

/*
 * Write a time value into a field
 */
static void write_time_value_to_field(int8_t id) {
  snprintf(value_text[id], sizeof(value_text[id]), "%02d", values[id]);
  text_layer_set_text(fields[id], value_text[id]);
}

/*
 * Write a ampm value into a field
 */
static void write_ampm_value_to_field(int8_t id) {
  text_layer_set_text(fields[id], values[id] == 0 ? TEXT_AM : TEXT_PM);
}

/*
 * Write values from memory into fields
 */
static void write_values_to_fields() {
  write_time_value_to_field(F_FROM_HOUR);
  write_time_value_to_field(F_FROM_MINUTE);
  write_time_value_to_field(F_TO_HOUR);
  write_time_value_to_field(F_TO_MINUTE);
  if (!is24hr) {
    write_ampm_value_to_field(F_FROM_AMPM);
    write_ampm_value_to_field(F_TO_AMPM);
  }
}

/*
 * Convert back to 24 hour clock
 */
static uint8_t hour_from_fields(int hour, int ampm) {
  if (is24hr) {
    return hour;
  } else {
    return (hour == 12 ? 0 : hour) + ampm * 12;
  }
}

/*
 * Drop values back into the rest of the system
 */
static void return_values() {
  get_config_data()->smart = true;
  get_config_data()->fromhr = hour_from_fields(values[F_FROM_HOUR],values[F_FROM_AMPM]);
  get_config_data()->frommin = values[F_FROM_MINUTE];
  get_config_data()->tohr = hour_from_fields(values[F_TO_HOUR],values[F_TO_AMPM]);
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
static void hide_set_alarm() {
  return_values();
  window_stack_remove(setting_window, true);
}

/*
 * Select button moves to next field - highlight and move
 */
static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  highlight_field(current_field, false);
  current_field++;
  if (is24hr && (current_field == F_FROM_AMPM || current_field == F_TO_AMPM)) {
    current_field++;
  }
  if (current_field > F_TO_AMPM) {
    hide_set_alarm();
    return;
  }
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
 * Handle an up or down button press
 */
static void up_down_handler(int8_t delta) {
  switch (current_field) {
    case F_FROM_HOUR:
    case F_TO_HOUR:
      alter_time(current_field, delta, is24hr ? 23 : 12, is24hr ? 0 : 1);
      break;
    case F_FROM_MINUTE:
    case F_TO_MINUTE:
      alter_time(current_field, 5 * delta, 55, 0);
      break;
    case F_FROM_AMPM:
    case F_TO_AMPM:
      alter_time(current_field, 1, 1, 0);
      break;
  }
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
  values[F_FROM_HOUR] = twenty_four_to_twelve(get_config_data()->fromhr);
  values[F_FROM_MINUTE] = get_config_data()->frommin;
  values[F_TO_HOUR] = twenty_four_to_twelve(get_config_data()->tohr);
  values[F_TO_MINUTE] = get_config_data()->tomin;
  if (!is24hr) {
    values[F_FROM_AMPM] = get_config_data()->fromhr >= 12 ? 1 : 0;
    values[F_TO_AMPM] = get_config_data()->tohr >= 12 ? 1 : 0;
  }
}

/*
 * Build the settings window
 */
static void create_settings_window(void) {
  setting_window = window_create();
  Layer *window_layer = window_get_root_layer(setting_window);
  GRect bounds = layer_get_bounds(window_layer);
  int16_t centre = bounds.size.w / 2;
  int16_t width = bounds.size.w;

  window_set_background_color(setting_window, SETTING_BACKGROUND_COLOR);
  
  is24hr = clock_is_24h_style();

  // Get the resources we need
  large_font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  small_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
 
  // Set click provider
  window_set_click_config_provider(setting_window, (ClickConfigProvider) setting_click_config_provider);

  // from_layer
  from_layer = macro_text_layer_create(GRect(SA_ROW_1_X, SA_ROW_1_Y, SA_ROW_1_WIDTH, SA_HEIGHT), window_layer, NON_HIGHLIGHT_FG_COLOR, SETTING_BACKGROUND_COLOR, small_font, GTextAlignmentCenter);
  text_layer_set_text(from_layer, EARLIEST);

  // fields[FROM_HOUR]
  fields[F_FROM_HOUR] = macro_text_layer_create(GRect(is24hr ? SA_HOUR_LEFT_24 : SA_HOUR_LEFT_12, SA_ROW_2_Y, SA_WIDTH, SA_HEIGHT), window_layer, NON_HIGHLIGHT_FG_COLOR, NON_HIGHLIGHT_BG_COLOR, large_font, GTextAlignmentCenter);

  // fields[FROM_MINUTE]
  fields[F_FROM_MINUTE] = macro_text_layer_create(GRect(is24hr ? SA_MIN_LEFT_24 : SA_MIN_LEFT_12, SA_ROW_2_Y, SA_WIDTH, SA_HEIGHT), window_layer, NON_HIGHLIGHT_FG_COLOR, NON_HIGHLIGHT_BG_COLOR, large_font, GTextAlignmentCenter);

  // fields[FROM_AMPM]
  if (!is24hr) {
    fields[F_FROM_AMPM] = macro_text_layer_create(GRect(SA_AMPM_LEFT, SA_ROW_2_Y, SA_WIDTH, SA_HEIGHT), window_layer, NON_HIGHLIGHT_FG_COLOR, NON_HIGHLIGHT_BG_COLOR, large_font, GTextAlignmentCenter);
  }
  
  // to_layer
  to_layer = macro_text_layer_create(GRect(SA_ROW_3_X, SA_ROW_3_Y, SA_ROW_3_WIDTH, SA_HEIGHT), window_layer, NON_HIGHLIGHT_FG_COLOR, SETTING_BACKGROUND_COLOR, small_font, GTextAlignmentCenter);
  text_layer_set_text(to_layer, LATEST);

  // fields[TO_HOUR]
  fields[F_TO_HOUR] = macro_text_layer_create(GRect(is24hr ? SA_HOUR_LEFT_24 : SA_HOUR_LEFT_12, SA_ROW_4_Y, SA_WIDTH, SA_HEIGHT), window_layer, NON_HIGHLIGHT_FG_COLOR, NON_HIGHLIGHT_BG_COLOR, large_font, GTextAlignmentCenter);

  // fields[TO_MINUTE]
  fields[F_TO_MINUTE] = macro_text_layer_create(GRect(is24hr ? SA_MIN_LEFT_24 : SA_MIN_LEFT_12, SA_ROW_4_Y, SA_WIDTH, SA_HEIGHT), window_layer, NON_HIGHLIGHT_FG_COLOR, NON_HIGHLIGHT_BG_COLOR, large_font, GTextAlignmentCenter);

  // fields[TO_AMPM]
  if (!is24hr) {
    fields[F_TO_AMPM] = macro_text_layer_create(GRect(SA_AMPM_LEFT, SA_ROW_4_Y, SA_WIDTH, SA_HEIGHT), window_layer, NON_HIGHLIGHT_FG_COLOR, NON_HIGHLIGHT_BG_COLOR, large_font, GTextAlignmentCenter);
  }
    
  current_field = F_FROM_HOUR;

  set_values();
  write_values_to_fields();
  highlight_field(current_field, true);
}

/*
 * Throw away the resources we gathered
 */
static void dispose_resources(void) {
  window_destroy(setting_window);
  text_layer_destroy(from_layer);
  text_layer_destroy(fields[F_FROM_HOUR]);
  text_layer_destroy(fields[F_FROM_MINUTE]);
  text_layer_destroy(to_layer);
  text_layer_destroy(fields[F_TO_HOUR]);
  text_layer_destroy(fields[F_TO_MINUTE]);
  if (!is24hr) {
     text_layer_destroy(fields[F_FROM_AMPM]); 
     text_layer_destroy(fields[F_TO_AMPM]); 
  }
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
EXTFN void show_set_alarm() {
  // Smart alarm on. Set some times.
  create_settings_window();
  window_set_window_handlers(setting_window, (WindowHandlers ) { .unload = handle_window_unload, });
  window_stack_push(setting_window, true);
}

