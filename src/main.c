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
#include "rootui.h"

// Shared with rootui, rectui, roundui, primary_window with main and notice_font with noticewindows
extern UiCommon ui;

// Make sure the voice is considered dead - we suspect vital processing for this, make sure it isn't in error
// this may be belt and braces, but testing suggests this can happen
static void show_main(Window *window) {
  #ifdef VOICE_SUPPORTED
    voice_system_inactive();
  #endif
}

/*
 * Create main window
 */
static void handle_init() {
  // Check which defines are defined
  #ifdef PBL_SDK_3
  LOG_INFO("PBL_SDK_3");
  #endif
  #ifdef PBL_PLATFORM_APLITE
  LOG_INFO("PBL_PLATFORM_APLITE");
  #endif
  #ifdef PBL_PLATFORM_BASALT
  LOG_INFO("PBL_PLATFORM_BASALT");
  #endif
  #ifdef PBL_PLATFORM_CHALK
  LOG_INFO("PBL_PLATFORM_CHALK");
  #endif
  #ifdef PBL_COLOR
  LOG_INFO("PBL_COLOR");
  #endif
  #ifdef PBL_BW
  LOG_INFO("PBL_BW");
  #endif
  #ifdef PBL_ROUND
  LOG_INFO("PBL_ROUND");
  #endif
  #ifdef PBL_RECT
  LOG_INFO("PBL_RECT");
  #endif
  
  // Create primary window
  ui.primary_window = window_create();
  window_set_window_handlers(ui.primary_window, (WindowHandlers ) { .load = morpheuz_load, .unload = morpheuz_unload, .appear = show_main });
  window_stack_push(ui.primary_window, true);
}

/**
 * Close main window
 */
EXTFN void close_morpheuz() {
  manual_shutdown_request();
  window_stack_remove(ui.primary_window, true);
  window_destroy(ui.primary_window);
}

/*
 * Main
 */
EXTFN int main(void) {
  handle_init();
  app_event_loop();
  lazarus();
}
