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

static AppTimer *keyboard_timer;
static AppTimer *notice_timer;

static BitmapLayer *arrow_l1_layer;
static BitmapLayer *arrow_r1_layer;
static BitmapLayer *arrow_r2_layer;
static BitmapLayer *arrow_r3_layer;
static BitmapLayer *keyboard_layer;
static BitmapLayer *notice_layer;
static BitmapLayer *notice_moon_layer;

static GBitmap *arrow_l_bitmap;
static GBitmap *arrow_r_bitmap;
static GBitmap *keyboard_bitmap;
static GBitmap *notice_moon_bitmap;

static GFont keyboard_time_font;
GFont notice_font;

static TextLayer *fatal_text;
static TextLayer *keyboard_time_text;
static TextLayer *l1_text;
static TextLayer *notice_name_layer;
static TextLayer *notice_text;
static TextLayer *r1_text;
static TextLayer *r2_text;
static TextLayer *r3_text;

static Window *fatal_window;
static Window *keyboard_window;
static Window *notice_window;

static bool fatal_showing = false;
static bool keyboard_showing = false;
static bool notice_showing = false;

static struct PropertyAnimation *moon_animation;

/*
 * Remove the keyboard window
 */
static void hide_keyboard_layer(void *data) {
  if (keyboard_showing) {
    window_stack_remove(keyboard_window, true);
    text_layer_destroy(keyboard_time_text);
    text_layer_destroy(r1_text);
    text_layer_destroy(r2_text);
    text_layer_destroy(r3_text);
    text_layer_destroy(l1_text);
    bitmap_layer_destroy(keyboard_layer);
    bitmap_layer_destroy(arrow_r1_layer);
    bitmap_layer_destroy(arrow_r2_layer);
    bitmap_layer_destroy(arrow_r3_layer);
    bitmap_layer_destroy(arrow_l1_layer);
    gbitmap_destroy(keyboard_bitmap);
    gbitmap_destroy(arrow_r_bitmap);
    gbitmap_destroy(arrow_l_bitmap);
    window_destroy(keyboard_window);
    keyboard_showing = false;
  }
}

/*
 * Show the keyboard window
 */
void show_keyboard() {

  if (fatal_showing)
    return;

  // It's night - want to see message and time
  light_enable_interaction();

  // Already showing - make it vanish quickly
  if (keyboard_showing) {
    app_timer_reschedule(keyboard_timer, 200);
    return;
  }

  bool invert = get_config_data()->invert;
  GColor fcolor = invert ? GColorBlack : GColorWhite;

  // Bring up notice
  keyboard_showing = true;
  keyboard_window = window_create();
  window_set_fullscreen(keyboard_window, true);
  window_stack_push(keyboard_window, true /* Animated */);
  window_set_background_color(keyboard_window, invert ? GColorWhite : GColorBlack);

  Layer *window_layer = window_get_root_layer(keyboard_window);

  keyboard_layer = bitmap_layer_create(GRect(6, 116, 58, 46));
  layer_add_child(window_layer, bitmap_layer_get_layer(keyboard_layer));
  keyboard_bitmap = gbitmap_create_with_resource(invert ? RESOURCE_ID_KEYBOARD_BG_WHITE : RESOURCE_ID_KEYBOARD_BG);
  bitmap_layer_set_bitmap(keyboard_layer, keyboard_bitmap);

  arrow_r_bitmap = gbitmap_create_with_resource(invert ? RESOURCE_ID_KEYBOARD_ARROW_R_BLACK : RESOURCE_ID_KEYBOARD_ARROW_R_WHITE);
  arrow_r1_layer = bitmap_layer_create(GRect(137, 43, 6, 8));
  layer_add_child(window_layer, bitmap_layer_get_layer(arrow_r1_layer));
  bitmap_layer_set_bitmap(arrow_r1_layer, arrow_r_bitmap);

  arrow_r2_layer = bitmap_layer_create(GRect(137, 88, 6, 8));
  layer_add_child(window_layer, bitmap_layer_get_layer(arrow_r2_layer));
  bitmap_layer_set_bitmap(arrow_r2_layer, arrow_r_bitmap);

  arrow_r3_layer = bitmap_layer_create(GRect(137, 136, 6, 8));
  layer_add_child(window_layer, bitmap_layer_get_layer(arrow_r3_layer));
  bitmap_layer_set_bitmap(arrow_r3_layer, arrow_r_bitmap);

  arrow_l_bitmap = gbitmap_create_with_resource(invert ? RESOURCE_ID_KEYBOARD_ARROW_L_BLACK : RESOURCE_ID_KEYBOARD_ARROW_L_WHITE);
  arrow_l1_layer = bitmap_layer_create(GRect(1, 43, 6, 8));
  layer_add_child(window_layer, bitmap_layer_get_layer(arrow_l1_layer));
  bitmap_layer_set_bitmap(arrow_l1_layer, arrow_l_bitmap);

  r1_text = macro_text_layer_create(GRect(44, 30, 90, 36),window_layer,fcolor, GColorClear, fonts_get_system_font(FONT_KEY_GOTHIC_14), GTextAlignmentRight);
  text_layer_set_text(r1_text, R1_TEXT);

  r2_text = macro_text_layer_create(GRect(44, 79, 90, 36),window_layer,fcolor, GColorClear, fonts_get_system_font(FONT_KEY_GOTHIC_14), GTextAlignmentRight);
  text_layer_set_text(r2_text, R2_TEXT);

  r3_text = macro_text_layer_create(GRect(44, 124, 90, 36),window_layer,fcolor, GColorClear, fonts_get_system_font(FONT_KEY_GOTHIC_14), GTextAlignmentRight);
  text_layer_set_text(r3_text, R3_TEXT);

  l1_text = macro_text_layer_create(GRect(10, 30, 90, 36),window_layer,fcolor, GColorClear, fonts_get_system_font(FONT_KEY_GOTHIC_14), GTextAlignmentLeft);
  text_layer_set_text(l1_text, L1_TEXT);

  keyboard_time_text = macro_text_layer_create(GRect(0, 0, 144, 30),window_layer,fcolor, GColorClear, keyboard_time_font, GTextAlignmentCenter);
  set_text_layer_to_time(keyboard_time_text);

  window_set_click_config_provider(keyboard_window, (ClickConfigProvider) click_config_provider);

  keyboard_timer = app_timer_register(KEYBOARD_DISPLAY_MS, hide_keyboard_layer, NULL);
}

/*
 * Remove the fatal window
 */
static void hide_fatal_layer() {
  if (fatal_showing) {
    window_stack_remove(fatal_window, true);
    text_layer_destroy(fatal_text);
    window_destroy(fatal_window);
    fatal_showing = false;
  }
}

/*
 * Show the fatal error window and do sos vibes
 */
void show_fatal(char *message) {

  if (fatal_showing)
    return;

  fatal_showing = true;

  // Bring up notice
  fatal_window = window_create();
  window_set_fullscreen(fatal_window, true);
  window_stack_push(fatal_window, false);
  window_set_background_color(fatal_window, GColorWhite);

  Layer *window_layer = window_get_root_layer(fatal_window);

  fatal_text = macro_text_layer_create(GRect(5, 5, 134, 158),window_layer,GColorBlack,GColorWhite, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GTextAlignmentLeft);
  text_layer_set_text(fatal_text, message);

  window_set_click_config_provider(fatal_window, (ClickConfigProvider) click_config_provider);

  do_vibes(VIBE_SOS);
}

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
    bitmap_layer_destroy(notice_layer);
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

  if (fatal_showing)
    return;

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
  keyboard_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_25));
}

void notice_deinit() {
  if (notice_showing)
    hide_notice_layer(NULL);
  if (keyboard_showing)
    hide_keyboard_layer(NULL);
  if (fatal_showing)
    hide_fatal_layer();
  fonts_unload_custom_font(notice_font);
  fonts_unload_custom_font(keyboard_time_font);
}
