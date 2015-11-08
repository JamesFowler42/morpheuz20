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
  
static char *text_pm_pad = TEXT_PM_PAD;
static char *text_am_pad = TEXT_AM_PAD;
static char *text_empty = "";

/*
 * Do all the poop associated with creating a text box, but in one lump
 */
EXTFN TextLayer* macro_text_layer_create(GRect frame, Layer *parent, GColor tcolor, GColor bcolor, GFont font, GTextAlignment text_alignment) {
  TextLayer* text_layer = text_layer_create(frame);
  text_layer_set_text_color(text_layer, tcolor);
  text_layer_set_background_color(text_layer, bcolor);
  text_layer_set_text_alignment(text_layer, text_alignment);
  text_layer_set_font(text_layer, font);
  layer_add_child(parent, text_layer_get_layer_jf(text_layer));
  return text_layer;
}

/*
 * Create a bitmap layer with bitmap in one go
 */
EXTFN void macro_bitmap_layer_create(BitmapLayerComp *comp, GRect frame, Layer *parent, uint32_t resource_id, bool visible) {
  comp->layer = bitmap_layer_create(frame);
#ifdef PBL_COLOR
  bitmap_layer_set_compositing_mode(comp->layer, GCompOpSet);
#endif
  layer_add_child(parent, bitmap_layer_get_layer_jf(comp->layer));
  comp->bitmap = gbitmap_create_with_resource(resource_id);
  bitmap_layer_set_bitmap(comp->layer, comp->bitmap);
  layer_set_hidden(bitmap_layer_get_layer_jf(comp->layer), !visible);
}

/*
 * Change the resource in a bitmap layer
 */
EXTFN void macro_bitmap_layer_change_resource(BitmapLayerComp *comp, uint32_t new_resource_id) {
  bitmap_layer_set_bitmap(comp->layer, NULL);
  gbitmap_destroy(comp->bitmap);
  comp->bitmap = gbitmap_create_with_resource(new_resource_id);
  bitmap_layer_set_bitmap(comp->layer, comp->bitmap);
}

/*
 * Get rid of the bitmap layer in one go
 */
EXTFN void macro_bitmap_layer_destroy(BitmapLayerComp *comp) {
  bitmap_layer_destroy(comp->layer);
  gbitmap_destroy(comp->bitmap);
}

/*
 * Updatable layer creation in one call
 */
EXTFN Layer * macro_layer_create(GRect frame, Layer *parent, LayerUpdateProc update_proc) {
  Layer *layer = layer_create(frame);
  layer_set_update_proc(layer, update_proc);
  layer_add_child(parent, layer);
  return layer;
}

/*
 * Combine two ints as a long
 */
EXTFN int32_t join_value(int16_t top, int16_t bottom) {
  int32_t top_as_32 = top;
  int32_t bottom_as_32 = bottom;
  return top_as_32 << 16 | bottom_as_32;
}

/*
 * Simple checksum routine
 */
EXTFN int32_t dirty_checksum(void *data, uint8_t data_size) {
  int16_t xor = 0xAAAA;
  int16_t sum = 0;
  uint8_t *d = data;
  for (uint8_t i = 0; i < data_size; i++) {
    int16_t val_d = *d;
    xor ^= val_d;
    sum += val_d;
    d++;
  }
  return join_value(xor, sum);
}

/*
 * Display the times using the settings the user prefers
 */
EXTFN uint8_t twenty_four_to_twelve(uint8_t hour) {
  if (!IS_24_HOUR_MODE) {
    return (hour == 0) ? 12 : (hour > 12) ? hour - 12 : hour;
  } else {
    return hour;
  }
}

/*
 * AM/PM indicator
 */
EXTFN char* am_pm_text(uint8_t hour) {
  if (!IS_24_HOUR_MODE) {
    return ((hour > 11) ? text_pm_pad : text_am_pad);
  } else {
    return text_empty;
  }
}

/*
 * Copy time range into field
 */
#ifndef PBL_PLATFORM_APLITE
EXTFN void copy_time_range_into_field(char *field, size_t fsize, uint8_t fromhr, uint8_t frommin, uint8_t tohr, uint8_t tomin ) {
  snprintf(field, fsize, MENU_SMART_ALARM_TIME_FORMAT, twenty_four_to_twelve(fromhr), frommin, am_pm_text(fromhr), twenty_four_to_twelve(tohr), tomin, am_pm_text(tohr));
}
#else
static void copy_time_range_into_field(char *field, size_t fsize, uint8_t fromhr, uint8_t frommin, uint8_t tohr, uint8_t tomin ) {
  snprintf(field, fsize, MENU_SMART_ALARM_TIME_FORMAT, twenty_four_to_twelve(fromhr), frommin, am_pm_text(fromhr), twenty_four_to_twelve(tohr), tomin, am_pm_text(tohr));
}
#endif  

/*
 * Copy alarm time range into field
 */
EXTFN void copy_alarm_time_range_into_field(char *field, size_t fsize) {
    copy_time_range_into_field(field,fsize, get_config_data()->fromhr, get_config_data()->frommin, get_config_data()->tohr, get_config_data()->tomin);
}

/*
 * Copy end alarm time into field
 */
#ifdef VOICE_SUPPORTED
EXTFN void copy_end_time_into_field(char *field, size_t fsize) {
  snprintf(field, fsize, MENU_SMART_ALARM_END_TIME_FORMAT, twenty_four_to_twelve(get_config_data()->tohr), get_config_data()->tomin, am_pm_text(get_config_data()->tohr));
}
#endif




