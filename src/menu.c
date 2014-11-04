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
#include "morpheuz.h"
#include "language.h"
#include "analogue.h"

#include "pebble.h"

#define NUM_MENU_SECTIONS 1
#define NUM_MENU_ICONS 2
#define NUM_FIRST_MENU_ITEMS 8

static Window *window;
static MenuLayer *menu_layer;
static GBitmap *menu_icons[NUM_MENU_ICONS];

static uint8_t ignore_state = 0;
static uint8_t weekend_state = 0;
static uint8_t inverse_state = 0;
static uint8_t analogue_state = 0;
static uint8_t power_nap_state = 0;
static bool alarm_on = false;
static char date_text[19];
static int16_t selected_row;

/*
 * A callback is used to specify the amount of sections of menu items
 * With this, you can dynamically add and remove sections
 */
static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

/*
 * Each section has a number of items;  we use a callback to specify this
 * You can also dynamically add and remove items using this
 */
static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
  case 0:
    return NUM_FIRST_MENU_ITEMS + (alarm_on ? 2 : 0);
  default:
    return 0;
  }
}

/*
 * A callback is used to specify the height of the section header
 */
static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

/*
 * Here we draw what each header is
 */
static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  time_t now = time(NULL);
  struct tm *time = localtime(&now);
  strftime(date_text, sizeof(date_text), "%B %e, %Y", time);

  switch (section_index) {
  case 0:
    menu_cell_basic_header_draw(ctx, cell_layer, date_text);
    break;
  }
}

/*
 * This is the menu item draw callback where you specify what each item should look like
 */
static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  int16_t row = alarm_on ? cell_index->row - 2 : cell_index->row;
  switch (cell_index->section) {
  case 0:
    switch (row) {
    case -2:
      menu_cell_basic_draw(ctx, cell_layer, MENU_SNOOZE, MENU_SNOOZE_DES, NULL);
      break;

    case -1:
      menu_cell_basic_draw(ctx, cell_layer, MENU_CANCEL, MENU_CANCEL_DES, NULL);
      break;

    case 0:
      menu_cell_basic_draw(ctx, cell_layer, MENU_IGNORE, MENU_IGNORE_DES, menu_icons[ignore_state]);
      break;

    case 1:
      menu_cell_basic_draw(ctx, cell_layer, MENU_RESET, MENU_RESET_DES, NULL);
      break;

    case 2:
      menu_cell_basic_draw(ctx, cell_layer, MENU_POWER_NAP, MENU_POWER_NAP_DES, menu_icons[power_nap_state]);
      break;

    case 3:
      menu_cell_basic_draw(ctx, cell_layer, MENU_WEEKEND, MENU_WEEKEND_DES, menu_icons[weekend_state]);
      break;

    case 4:
      menu_cell_basic_draw(ctx, cell_layer, MENU_RESEND, MENU_RESEND_DES, NULL);
      break;

    case 5:
      menu_cell_basic_draw(ctx, cell_layer, MENU_INVERSE, MENU_INVERSE_DES, menu_icons[inverse_state]);
      break;

    case 6:
      menu_cell_basic_draw(ctx, cell_layer, MENU_ANALOGUE, MENU_ANALOGUE_DES, menu_icons[analogue_state]);
      break;

    case 7:
      menu_cell_basic_draw(ctx, cell_layer, MENU_QUIT, MENU_QUIT_DES, NULL);
      break;
    }
    break;
  }
}

/*
 * Do menu action after shutting the menu and allowing time for the animations to complete
 */
static void do_menu_action(void *data) {
  switch (selected_row) {
  case -2:
    snooze_alarm();
    break;
  case -1:
    cancel_alarm();
    break;
  case 0:
    set_ignore_on_current_time_segment();
    break;
  case 1:
    reset_sleep_period();
    break;
  case 2:
    toggle_power_nap();
    break;
  case 3:
    toggle_weekend_mode();
    break;
  case 4:
    resend_all_data();
    break;
  case 5:
    set_config_data_invert(!get_config_data()->invert);
    invert_screen();
    break;
  case 6:
    set_config_data_analogue(!get_config_data()->analogue);
    analogue_visible(get_config_data()->analogue);
    break;
  case 7:
    close_morpheuz();
    break;
  }
}

/*
 * Here we capture when a user selects a menu item
 */
void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  // Use the row to specify which item will receive the select action
  selected_row = alarm_on ? cell_index->row - 2 : cell_index->row;
  hide_menu();
  app_timer_register(MENU_ACTION_MS, do_menu_action, NULL);
}

/*
 * This initializes the menu upon window load
 */
void window_load(Window *window) {
  int num_menu_icons = 0;
  menu_icons[num_menu_icons++] = gbitmap_create_with_resource(RESOURCE_ID_MENU_NO);
  menu_icons[num_menu_icons++] = gbitmap_create_with_resource(RESOURCE_ID_MENU_YES);

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  menu_layer = menu_layer_create(bounds);

  menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks ) { .get_num_sections = menu_get_num_sections_callback, .get_num_rows = menu_get_num_rows_callback, .get_header_height = menu_get_header_height_callback, .draw_header = menu_draw_header_callback, .draw_row = menu_draw_row_callback, .select_click = menu_select_callback, });

  menu_layer_set_click_config_onto_window(menu_layer, window);

  layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
}

/*
 * Unload the menu window
 */
void window_unload(Window *window) {
  menu_layer_destroy(menu_layer);

  for (int i = 0; i < NUM_MENU_ICONS; i++) {
    gbitmap_destroy(menu_icons[i]);
  }

}

/*
 * Show the menu
 */
void show_menu(bool ignore, bool weekend, bool inverse, bool analogue, bool power_nap, bool alarm) {
  ignore_state = ignore ? 1 : 0;
  weekend_state = weekend ? 1 : 0;
  inverse_state = inverse ? 1 : 0;
  analogue_state = analogue ? 1 : 0;
  power_nap_state = power_nap ? 1 : 0;
  alarm_on = alarm;
  window = window_create();
  // Setup the window handlers
  window_set_window_handlers(window, (WindowHandlers ) { .load = window_load, .unload = window_unload, });
  window_stack_push(window, true /* Animated */);
}

/*
 * Hide the menu (destroy)
 */
void hide_menu() {
  window_stack_remove(window, true);
  window_destroy(window);
}

