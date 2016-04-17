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
 * The above copyright chart and this permission chart shall be included in
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
#include "chart.h"

#ifdef ENABLE_CHART_VIEWER

#include "rootui.h"

#define MOON_START GRect(width+6, 72, 58, 46)  
#define MOON_FINISH PBL_IF_ROUND_ELSE(GRect(centre - 29, 5, 58, 46), GRect(6, 5, 58, 46))

VERSION_EXTERNAL;

static AppTimer *chart_timer;

static BitmapLayerComp chart_moon;

// Shared with rootui, rectui, roundui, primary_window with main and chart_font with chartwindows
extern UiCommon ui;

#ifndef PBL_ROUND
static TextLayer *chart_name_layer;
#endif

static TextLayer *chart_text;

static Window *chart_window;

static bool chart_showing = false;

static struct PropertyAnimation *moon_animation;

static ChartData chart_data;

static Layer *bar_layer;
static TextLayer *chart_date;

static int16_t bar_height;
static int16_t bar_width;

static char date_text[25];

#ifdef TESTING_BUILD
static int16_t dummy_data[] = { 1835, 2300, 775, 806, 1112, -2, 1142, 826, 815, 2210, 1190, 998, 1053, 1388, 1177, 1033, 1532, 1366, 96, 147, 2736, 310, 92, 1806, 790, 992, 33, 2174, 382, 117, 519, 177, 452, 690, 532, 773, 878, 1413, 1175, 1187, 863, 223, 1805, 960, 83, 2053, 1050, 484, 913, 784, 1784, 54, -1, -1, -1, -1, -1, -1, -1, -1 };
#endif

static void reset_chart_data() {
  
  #ifdef TESTING_BUILD
  for (int i = 0; i < LIMIT; i++) {
    if (dummy_data[i] != -2) {
      if (dummy_data[i] != -1) {
      chart_data.points[i] = dummy_data[i];
      } else {
        chart_data.points[i] = 0;
      }
      chart_data.ignore[i] = false;
    } else {
      chart_data.points[i] = 0;
      chart_data.ignore[i] = true;
    }
  }
  chart_data.highest_entry = 59;
  chart_data.base = 1460759878;
  chart_data.gone_off = 404;
  chart_data.from = 390;
  chart_data.to = 435;
  chart_data.smart = true;
  #else
  chart_data.base = 0;
  #endif
}

/*
 * Read the chart data
 */
static void read_chart_data() {
  int read = persist_read_data(PERSIST_CHART_KEY, &chart_data, sizeof(chart_data));
  if (read != sizeof(chart_data) || chart_data.chart_ver != CHART_VER) {
    reset_chart_data();
  }
}

/*
 * Save the chart data structure
 */
static void save_chart_data() {
  LOG_DEBUG("save_chart_data (%d)", sizeof(chart_data));
  int written = persist_write_data(PERSIST_CHART_KEY, &chart_data, sizeof(chart_data));
  if (written != sizeof(chart_data)) {
    LOG_ERROR("save_chart_data error (%d)", written);
  }
}

/*
 * Store the current information at this point in time - keep it until replaced
 */
EXTFN void store_chart_data() {
  chart_data.chart_ver = CHART_VER;
  chart_data.base = get_internal_data()->base;
  chart_data.gone_off = get_internal_data()->gone_off;
  chart_data.highest_entry = get_internal_data()->highest_entry;
  for (int i = 0; i < LIMIT; i++) {
    chart_data.points[i] = get_internal_data()->points[i];
    chart_data.ignore[i] = get_internal_data()->ignore[i];
  }
  chart_data.snoozes = get_internal_data()->snoozes;
  chart_data.from = get_config_data()->from;
  chart_data.to = get_config_data()->to;
  chart_data.smart = get_config_data()->smart;
  save_chart_data();
}

/*
 * Remove the chart window
 */
EXTFN void hide_chart_layer(void *data) {
  if (chart_showing) {
    window_stack_remove(chart_window, true);
    macro_bitmap_layer_destroy(&chart_moon);
#ifndef PBL_ROUND
    text_layer_destroy(chart_name_layer);
#endif
    text_layer_destroy(chart_text);
    layer_destroy(bar_layer);
    window_destroy(chart_window);
    chart_showing = false;
  }
}

/*
 * End of chart window animation
 */
static void moon_animation_stopped(Animation *animation, bool finished, void *data) {
}

/*
 * Click a button on a chart and it goes away
 */
static void single_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (chart_showing) {
    app_timer_reschedule(chart_timer, SHORT_RETRY_MS);
  }
}

/*
 * Button config
 */
static void chart_click_config_provider(Window *window) {
  window_single_click_subscribe(BUTTON_ID_BACK, single_click_handler);
}

/*
 * Advance by one day for each segment, looping back at midnight
 */
uint32_t next_after(uint32_t before) {
  uint32_t after = before + MINS_PER_SEGMENT;
  if (after > MINS_IN_DAY) {
    after -= MINS_IN_DAY;
  }
  return after;
}

/*
 * Work out the drawing width
 */
static int32_t calc_stroke_width() {
  return ((int32_t) bar_width) / ((int32_t) LIMIT);
}

/*
 * Calculate the left given screen size and position of data
 */
static int32_t x_from_position(uint8_t position) {
  int32_t x = (((int32_t) bar_width) * ((int32_t) position)) / ((int32_t) LIMIT);
  return x;
}

/*
 * Draw a bar for a particular 10 minute slice
 */
static void draw_bar_sector(GContext *ctx, GColor color, uint8_t position, int32_t stroke_width) {
  int32_t x = x_from_position(position);

  graphics_context_set_stroke_width(ctx, stroke_width);
  graphics_context_set_stroke_color(ctx, color);
  graphics_draw_line(ctx, GPoint(x, 0), GPoint(x, bar_height-7));
}

/*
 * Update the chart
 */
static void bar_layer_update_callback(Layer *layer, GContext *ctx) {

  // Fill background
  graphics_context_set_fill_color(ctx, CHART_BACKGROUND_COLOR);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
  
  graphics_context_set_fill_color(ctx, CHART_INDICATOR_BACKGROUND_COLOR);
  graphics_fill_rect(ctx, GRect(0,bar_height - 5, bar_width, 5), 0, GCornerNone);
  
  // Only do this if there is a chart to display
  if (chart_data.base != 0) {
    
    int32_t stroke_width = calc_stroke_width();

    // Paint restless, light, deep and ignore
    for (uint8_t i = 0; i <= chart_data.highest_entry; i++) {
      if (!chart_data.ignore[i]) {
        uint16_t height = chart_data.points[i];
        if (height > AWAKE_ABOVE) {
          draw_bar_sector(ctx, CHART_AWAKE_COLOR, i, stroke_width);
        } else if (height > LIGHT_ABOVE) {
          draw_bar_sector(ctx, CHART_LIGHT_COLOR, i, stroke_width);
        } else if (height > 0) {
          draw_bar_sector(ctx, CHART_DEEP_COLOR, i, stroke_width);
        }
      } else {
        draw_bar_sector(ctx, CHART_IGNORE_COLOR, i, stroke_width);
      }
    }
  
    // Remember the positions of the gone to sleep and woke up markers
    int8_t gone_to_sleep_i = 0;
    int8_t woke_up_i = 0;

    // Work out the gone to sleep position
    for (uint8_t i = 0; i <= chart_data.highest_entry; i++) {
      if (!chart_data.ignore[i]) {
        if (chart_data.points[i] <= AWAKE_ABOVE && i < chart_data.highest_entry) {
          gone_to_sleep_i = i;
          break;
        }
      }
    }

    // Calculate base as a time
    time_t base = chart_data.base;
    struct tm *time = localtime(&base);

    // Strange to do this here, but we have the data so might as well fill in the text too
    strftime(date_text, sizeof(date_text), clock_is_24h_style() ? DATE_TIME_FORMAT_24 : DATE_TIME_FORMAT_12, time);
    text_layer_set_text(chart_date, date_text);

    uint32_t base_hrs_mins = to_mins(time->tm_hour, time->tm_min);

    uint32_t before = 0;

    // Only bother with the smart alarm markers if there actually was one set
    if (chart_data.smart) {
      
      // Paint smart alarm start blobby
      before = base_hrs_mins;
      for (uint8_t i = 0; i <= LIMIT; i++) {
        uint32_t after = next_after(before);
        if (chart_data.from >= before && chart_data.from <= after) {
          int32_t early_left = x_from_position(i) - stroke_width;
          graphics_context_set_stroke_color(ctx, CHART_SLEEP_SMART_EARLIEST_COLOR);
          graphics_context_set_fill_color(ctx, CHART_SLEEP_SMART_EARLIEST_COLOR);
          graphics_fill_rect(ctx, GRect(early_left,bar_height - 5, stroke_width * 2, 5), 0, GCornerNone);
          break;
        }
        before = after;
      }

      // Paint smart alarm end blobby
      before = base_hrs_mins;
      for (uint8_t i = 0; i <= LIMIT; i++) {
        uint32_t after = next_after(before);
        if (chart_data.to >= before && chart_data.to <= after) {
          int32_t late_left = x_from_position(i) - stroke_width;
          graphics_context_set_stroke_color(ctx, CHART_SLEEP_SMART_LATEST_COLOR);
          graphics_context_set_fill_color(ctx, CHART_SLEEP_SMART_LATEST_COLOR);
          graphics_fill_rect(ctx, GRect(late_left,bar_height - 5, stroke_width * 2, 5), 0, GCornerNone);
          break;
        }
        before = after;
      }
    }

    // Work out the wake up point
    if (chart_data.gone_off != 0) {
    
      // First way - if the alarm has gone off then we can assume this is it
      before = base_hrs_mins;
      for (uint8_t i = 0; i <= LIMIT; i++) {
        uint32_t after = next_after(before);
        if (chart_data.gone_off >= before && chart_data.gone_off <= after) {
          woke_up_i = i;
          break;
        }
        before = after;
      }
    } else {
      
      // Otherwise calculate back from the end until we get something over the awake level
      for (uint8_t i = chart_data.highest_entry; i > 0; i--) {
        if (!chart_data.ignore[i]) {
          if (chart_data.points[i] > AWAKE_ABOVE) {
            woke_up_i = i;
            break;
          }
        }  
      }
    }
  
    // Now is a good time to record a horizontal bar for time spent asleep
    graphics_context_set_stroke_width(ctx, 1);
    int32_t sleep_left = x_from_position(gone_to_sleep_i);
    int32_t sleep_width = x_from_position(woke_up_i) - sleep_left + stroke_width;

    graphics_context_set_stroke_color(ctx, CHART_SLEEP_AWAKE_MARKER_COLOR);
    graphics_context_set_fill_color(ctx, CHART_SLEEP_AWAKE_MARKER_COLOR);
    graphics_fill_rect(ctx, GRect(sleep_left,bar_height - 4, sleep_width, 2), 0, GCornerNone);

  } else {
    
    // Or there was no data
    text_layer_set_text(chart_date, NO_CHART_RECORDED);
  }

  // Add trim
  graphics_context_set_stroke_width(ctx, 1);
  graphics_context_set_stroke_color(ctx, CHART_TRIM_COLOR);
  graphics_draw_line(ctx, GPoint(0, 0), GPoint(bar_width, 0));
  graphics_draw_line(ctx, GPoint(0, bar_height - 1), GPoint(bar_width, bar_height - 1));
  graphics_draw_line(ctx, GPoint(0, bar_height - 6), GPoint(bar_width, bar_height - 6));
}

/*
 * Show the chart window
 */
EXTFN void show_chart() {

  // Bring up chart
  chart_showing = true;
  read_chart_data();

  chart_window = window_create();
  window_stack_push(chart_window, true);

  window_set_background_color(chart_window, BACKGROUND_COLOR);

  Layer *window_layer = window_get_root_layer(chart_window);

  GRect bounds = layer_get_bounds(window_layer);
  uint16_t width = bounds.size.w;
#ifdef PBL_ROUND
  int16_t centre = bounds.size.w / 2;
#endif

  macro_bitmap_layer_create(&chart_moon, MOON_START, window_layer, RESOURCE_ID_KEYBOARD_BG, true);

#ifndef PBL_ROUND
  chart_name_layer = macro_text_layer_create(GRect(5, 15, 134, 30), window_layer, GColorWhite, GColorClear, ui.notice_font, GTextAlignmentRight);
  text_layer_set_text(chart_name_layer, APP_NAME);
#endif

  window_set_click_config_provider(chart_window, (ClickConfigProvider) chart_click_config_provider);

  int16_t v_centre = bounds.size.h / 2;
  int16_t h_centre = bounds.size.w / 2;
  bar_height = bounds.size.h * 2 / 5;
  int16_t bar_top = v_centre - bar_height / 2 + 5;
  bar_width = bounds.size.w;
  int16_t bar_left = h_centre - bar_width / 2;

  chart_date = macro_text_layer_create(GRect(0, bar_top + bar_height - 2, width, 31), window_layer, GColorWhite, BACKGROUND_COLOR, fonts_get_system_font(FONT_KEY_GOTHIC_24), GTextAlignmentCenter);

  bar_layer = macro_layer_create(GRect(bar_left, bar_top, bar_width, bar_height), window_layer, &bar_layer_update_callback);

  moon_animation = property_animation_create_layer_frame(bitmap_layer_get_layer_jf(chart_moon.layer), &MOON_START, &MOON_FINISH);
  animation_set_duration((Animation*) moon_animation, 750);
  animation_set_handlers((Animation*) moon_animation, (AnimationHandlers ) { .stopped = (AnimationStoppedHandler) moon_animation_stopped, }, NULL /* callback data */);

  animation_schedule((Animation*) moon_animation);

  chart_timer = app_timer_register(CHART_DISPLAY_MS, hide_chart_layer, NULL);
}

/*
 * Provide a hook so as the rest of the system can know not to interupt a chart
 */
EXTFN bool is_chart_showing() {
  return chart_showing;
}

#endif
