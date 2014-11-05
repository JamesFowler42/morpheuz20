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

static AppTimer *notice_timer;

static BitmapLayer *notice_moon_layer;

static GBitmap *notice_moon_bitmap;

GFont notice_font;

static TextLayer *notice_name_layer;
static TextLayer *notice_text;

static Window *notice_window;

static bool notice_showing = false;

static struct PropertyAnimation *moon_animation;

/*
 * Remove the notice window
 */
static void hide_notice_layer(void *data) {
  if (notice_showing) {
    window_stack_remove(notice_window, true);
    bitmap_layer_destroy(notice_moon_layer);
    gbitmap_destroy(notice_moon_bitmap);
    text_layer_destroy(notice_name_layer);
    text_layer_destroy(notice_text);
    window_destroy(notice_window);
    notice_showing = false;
  }
}

/*
 * End of notice window animation
 */
static void moon_animation_stopped(Animation *animation, bool finished, void *data) {
    animation_unschedule((Animation*) moon_animation);
    animation_destroy((Animation*) moon_animation);
}

/*
 * Show the notice window
 */
void show_notice(char *message) {

  // It's night - want to see message
  light_enable_interaction();

  // Already showing - change message
  if (notice_showing) {
    text_layer_set_text(notice_text, message);
    app_timer_reschedule(notice_timer, NOTICE_DISPLAY_MS);
    return;
  }

  // Bring up notice
  notice_showing = true;
  notice_window = window_create();
  window_set_fullscreen(notice_window, true);
  window_stack_push(notice_window, true /* Animated */);

  bool invert = get_config_data()->invert;
  GColor fcolor = invert ? GColorBlack : GColorWhite;

  window_set_background_color(notice_window, invert ? GColorWhite : GColorBlack);

  Layer *window_layer = window_get_root_layer(notice_window);

  notice_moon_layer = bitmap_layer_create(MOON_START);
  layer_add_child(window_layer, bitmap_layer_get_layer(notice_moon_layer));
  notice_moon_bitmap = gbitmap_create_with_resource(invert ? RESOURCE_ID_KEYBOARD_BG_WHITE : RESOURCE_ID_KEYBOARD_BG);
  bitmap_layer_set_bitmap(notice_moon_layer, notice_moon_bitmap);

  notice_name_layer = macro_text_layer_create(GRect(5, 15, 134, 30),window_layer,fcolor, GColorClear, notice_font, GTextAlignmentRight);
  text_layer_set_text(notice_name_layer, MORPHEUZ);

  notice_text = macro_text_layer_create(GRect(4, 68, 136, 100),window_layer,fcolor, GColorClear, notice_font, GTextAlignmentLeft);
  text_layer_set_text(notice_text, message);

  window_set_click_config_provider(notice_window, (ClickConfigProvider) click_config_provider);

  GRect start = MOON_START;
  GRect finish = MOON_FINISH;
  moon_animation = property_animation_create_layer_frame(bitmap_layer_get_layer(notice_moon_layer), &start, &finish);
  animation_set_duration((Animation*) moon_animation, 750);
  animation_set_handlers((Animation*) moon_animation, (AnimationHandlers ) { .stopped = (AnimationStoppedHandler) moon_animation_stopped, }, NULL /* callback data */);

  animation_schedule((Animation*) moon_animation);

  notice_timer = app_timer_register(NOTICE_DISPLAY_MS, hide_notice_layer, NULL);
}

void notice_init() {
  notice_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_16));
}

bool is_notice_showing() {
  return notice_showing;
}

void notice_deinit() {
  if (notice_showing)
    hide_notice_layer(NULL);
  fonts_unload_custom_font(notice_font);
}
