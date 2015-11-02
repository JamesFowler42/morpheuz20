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

#ifndef ROOT_UI_H_
#define ROOT_UI_H_

#include "pebble.h"
#include "morpheuz.h"

// Shared structure with rootui, rectui, roundui, primary_window with main and notice_font with noticewindows
typedef struct {
  Window *primary_window;
  Layer *icon_bar;
  TextLayer *text_date_smart_alarm_range_layer;
  uint8_t animation_count;
  TextLayer *powernap_layer;
  #ifdef PBL_COLOR
  TextLayer *text_time_shadow_layer;
  #endif 
  TextLayer *text_time_layer;
  uint8_t battery_level;
  bool battery_plugged;
  BitmapLayerComp alarm_button_top;
  BitmapLayerComp alarm_button_button;
  Layer *progress_layer;
  GFont notice_font;
  GFont time_font;
  TextLayer *version_text;
} UiCommon;

#endif

