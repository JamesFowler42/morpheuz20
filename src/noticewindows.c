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

static AppTimer *notice_timer;

static BitmapLayerComp notice_moon;

extern GFont notice_font;

static TextLayer *notice_name_layer;
static TextLayer *notice_text;

static Window *notice_window;

static bool notice_showing = false;

static struct PropertyAnimation *moon_animation;

static char *buffer;

/*
 * Remove the notice window
 */
void hide_notice_layer(void *data) {
  if (notice_showing) {
    window_stack_remove(notice_window, true);
    macro_bitmap_layer_destroy(&notice_moon);
    text_layer_destroy(notice_name_layer);
    text_layer_destroy(notice_text);
    window_destroy(notice_window);
    free(buffer);
    notice_showing = false;
  }
}

/*
 * End of notice window animation
 */
static void moon_animation_stopped(Animation *animation, bool finished, void *data) {
#ifndef PBL_COLOR
  animation_unschedule(animation);
  animation_destroy(animation);
#endif
}

static void load_resource_into_buffer(uint32_t resource_id) {
  ResHandle rh = resource_get_handle(resource_id);
  resource_load(rh, (uint8_t *) buffer, BUFFER_SIZE);
  text_layer_set_text(notice_text, buffer);
}

/*
 * Show the notice window
 */
void show_notice(uint32_t resource_id) {

  // It's night - want to see message
  light_enable_interaction();

  // Already showing - change message
  if (notice_showing) {
    load_resource_into_buffer(resource_id);
    app_timer_reschedule(notice_timer, NOTICE_DISPLAY_MS);
    return;
  }

  buffer = malloc(BUFFER_SIZE);

  // Bring up notice
  notice_showing = true;
  notice_window = window_create();
#ifndef PBL_COLOR
  window_set_fullscreen(notice_window, true);
#endif
  window_stack_push(notice_window, true /* Animated */);

  bool invert = get_config_data()->invert;
  GColor fcolor = invert ? GColorBlack : GColorWhite;

  window_set_background_color(notice_window, invert ? GColorWhite : BACKGROUND_COLOR);

  Layer *window_layer = window_get_root_layer(notice_window);

  macro_bitmap_layer_create(&notice_moon, MOON_START, window_layer, invert ? RESOURCE_ID_KEYBOARD_BG_WHITE : RESOURCE_ID_KEYBOARD_BG, true);

  notice_name_layer = macro_text_layer_create(GRect(5, 15, 134, 30), window_layer, fcolor, GColorClear, notice_font, GTextAlignmentRight);
  text_layer_set_text(notice_name_layer, MORPHEUZ);

  notice_text = macro_text_layer_create(GRect(4, 68, 136, 100), window_layer, fcolor, GColorClear, notice_font, GTextAlignmentLeft);
  load_resource_into_buffer(resource_id);

  window_set_click_config_provider(notice_window, (ClickConfigProvider) click_config_provider);

  moon_animation = property_animation_create_layer_frame(bitmap_layer_get_layer_jf(notice_moon.layer), &MOON_START, &MOON_FINISH);
  animation_set_duration((Animation*) moon_animation, 750);
  animation_set_handlers((Animation*) moon_animation, (AnimationHandlers ) { .stopped = (AnimationStoppedHandler) moon_animation_stopped, }, NULL /* callback data */);

  animation_schedule((Animation*) moon_animation);

  notice_timer = app_timer_register(NOTICE_DISPLAY_MS, hide_notice_layer, NULL);
}

bool is_notice_showing() {
  return notice_showing;
}

