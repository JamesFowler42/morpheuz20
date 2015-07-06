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
#include "analogue.h"

static bool power_nap_mode = false;
static int16_t power_nap_minute_count = 0;
static uint8_t power_nap_settle_count = 0;

/*
 * Power nap reset
 */
void power_nap_reset() {
  power_nap_mode = false;
  analogue_powernap_text(POWER_NAP_OFF_INDICATOR);
}

/*
 * Power nap settle check
 */
void power_nap_check(uint16_t biggest) {

  if (!power_nap_mode)
    return;

  if (power_nap_settle_count == 0)
    return;

  if (biggest < POWER_NAP_SETTLE_THRESHOLD)
    power_nap_settle_count--;
}

/*
 * Power nap countdown
 */
void power_nap_countdown() {

  if (!power_nap_mode)
    return;

  if (power_nap_settle_count != 0)
    return;

  if (power_nap_minute_count == 0)
    return;

  power_nap_minute_count--;

  char power_nap_ind[3];
  snprintf(power_nap_ind, sizeof(power_nap_ind), "%d", power_nap_minute_count);
  analogue_powernap_text(power_nap_ind);

  if (power_nap_minute_count == 0)
    fire_alarm();
}

/**
 * power nap
 */
void toggle_power_nap() {
  // Toggle sleep
  if (power_nap_mode) {
    // Turn off power nap
    power_nap_reset();
  } else {
    // Turn on power nap
    power_nap_mode = true;
    power_nap_minute_count = POWER_NAP_MINUTES + 1;
    power_nap_settle_count = POWER_NAP_SETTLE;
    analogue_powernap_text(POWER_NAP_SETTLE_INDICATOR);
  }
}

/**
 * Back button single click handler
 */
static void back_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Stop accidental closure of Morpheuz by defining this
  // Bring clock up to date if a button is pressed
  // Only if we're recording or running powernap
  if (get_icon(IS_RECORD) || is_doing_powernap()) {
    revive_clock_on_movement(CLOCK_UPDATE_THRESHOLD);
  } else {
    close_morpheuz();  
  }
}

/*
 * Single click handler on down button
 */
static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Make the snooze and the cancel buttons the same way around as the default alarm app
  #ifdef PBL_COLOR
    cancel_alarm();
  #else
    snooze_alarm();
  #endif
  // Bring clock up to date if a button is pressed
  revive_clock_on_movement(CLOCK_UPDATE_THRESHOLD);
}

/*
 * Single click handler on select button
 */
static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Bring clock up to date if a button is pressed
  revive_clock_on_movement(CLOCK_UPDATE_THRESHOLD);
  if (!is_notice_showing())
    show_menu();
}

bool is_doing_powernap() {
  return power_nap_mode;
}

/*
 * Single click handler on up button
 */
static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Make the snooze and the cancel buttons the same way around as the default alarm app
  #ifdef PBL_COLOR
    snooze_alarm();
  #else
    cancel_alarm();
  #endif
  // Bring clock up to date if a button is pressed
  revive_clock_on_movement(CLOCK_UPDATE_THRESHOLD);
}

/*
 * Button config
 */
void click_config_provider(Window *window) {
  window_single_click_subscribe(BUTTON_ID_BACK, back_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);
}
