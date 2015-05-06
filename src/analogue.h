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

#ifndef ANALOGUE_H_
#define ANALOGUE_H_

#include "pebble.h"

#define ANALOGUE_FINISH GRect(0, 24, 144, 144)
#define ANALOGUE_START GRect(0, 169, 144, 144)

#define TEXT_12 "12"
#define TEXT_3 "3"
#define TEXT_6 "6"
#define TEXT_9 "9"

#define OUTER 0
#define OUTER_STOP (OUTER + 4)
#define PROGRESS (OUTER_STOP + 3)
#define PROGRESS_STOP (PROGRESS + 1)
#define CLOCK (PROGRESS)
#define HOUR (CLOCK + 14)
#define MIN (CLOCK + 4)

void analogue_window_load(Window *window);
void analogue_minute_tick();
void analogue_set_smart_times();
void analogue_set_base(time_t base);
void analogue_set_progress(uint8_t progress_level_in);
void analogue_window_unload();
void analogue_visible(bool visible, bool call_post_init);
void analogue_powernap_text(char *text);
void set_weekend_icon(bool weekend);

#endif /* ANALOGUE_H_ */
