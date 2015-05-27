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

/*
 * Do all the poop associated with creating a text box, but in one lump
 */
TextLayer* macro_text_layer_create(GRect frame, Layer *parent, GColor tcolor, GColor bcolor, GFont font, GTextAlignment text_alignment) {
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
void macro_bitmap_layer_create(BitmapLayerComp *comp, GRect frame, Layer *parent, uint32_t resource_id, bool visible) {
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
 * Get rid of the bitmap layer in one go
 */
void macro_bitmap_layer_destroy(BitmapLayerComp *comp) {
  bitmap_layer_destroy(comp->layer);
  gbitmap_destroy(comp->bitmap);
}

/*
 * Combine two ints as a long
 */
int32_t join_value(int16_t top, int16_t bottom) {
  int32_t top_as_32 = top;
  int32_t bottom_as_32 = bottom;
  return top_as_32 << 16 | bottom_as_32;
}

/*
 * Simple checksum routine
 */
int32_t dirty_checksum(void *data, uint8_t data_size) {
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
uint8_t twenty_four_to_twelve(uint8_t hour) {
  return (hour <= 12 || clock_is_24h_style()) ? hour : hour - 12;
}

