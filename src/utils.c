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

/*
 * Set a text layer to the current time applying silly little corrections
 */
void set_text_layer_to_time(TextLayer *text_layer) {
  static char time_text[6];
  clock_copy_time_string  (time_text, sizeof(time_text));
  if (time_text[4] == ' ')
    time_text[4] = '\0';
  text_layer_set_text(text_layer, time_text);
}

/*
 * Do all the poop associated with creating a text box, but in one lump
 */
TextLayer* macro_text_layer_create(GRect frame, Layer *parent, GColor tcolor, GColor bcolor, GFont font, GTextAlignment text_alignment) {
  TextLayer* text_layer = text_layer_create(frame);
  text_layer_set_text_color(text_layer, tcolor);
  text_layer_set_background_color(text_layer, bcolor);
  text_layer_set_text_alignment(text_layer, text_alignment);
  text_layer_set_font(text_layer, font);
  layer_add_child(parent, text_layer_get_layer(text_layer));
  return text_layer;
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
