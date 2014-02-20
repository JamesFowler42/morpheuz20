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

static bool power_nap_mode = false;
static int16_t power_nap_minute_count = 0;
static uint8_t power_nap_settle_count = 0;

/*
 * Power nap reset
 */
void power_nap_reset() {
	power_nap_mode = false;
	set_smart_status();
}

/*
 * Power nap settle check
 */
void power_nap_check(uint16_t biggest) {

	if (!power_nap_mode) return;

	if (power_nap_settle_count == 0) return;

	if (biggest < POWER_NAP_SETTLE_THRESHOLD)
		power_nap_settle_count--;
}

/*
 * Power nap countdown
 */
void power_nap_countdown() {

	if (!power_nap_mode) return;

	if (power_nap_settle_count != 0) return;

	if (power_nap_minute_count == 0) return;

	power_nap_minute_count--;

	char power_nap_text[15];
	snprintf(power_nap_text, sizeof(power_nap_text), POWER_NAP_RUNNING, power_nap_minute_count);
	set_smart_status_on_screen(true, power_nap_text);

	if (power_nap_minute_count == 0)
		fire_alarm();
}

/**
 * Select button click down handler (power nap)
 */
static void select_long_down_handler(ClickRecognizerRef recognizer, void *context) {
	// Toggle sleep
	if (power_nap_mode) {
		// Turn off power nap
		power_nap_reset();
		show_notice(NOTICE_STOPPED_POWER_NAP);
	} else {
		// Turn on power nap
		power_nap_mode = true;
		power_nap_minute_count = POWER_NAP_MINUTES + 1;
		power_nap_settle_count = POWER_NAP_SETTLE;
		set_smart_status_on_screen(true, POWER_NAP_SETTLE_TIME);
		show_notice(NOTICE_STARTED_POWER_NAP);
	}
}

/**
 * Select button click up handler (power nap)
 */
static void select_long_up_handler(ClickRecognizerRef recognizer, void *context) {
	// Take no action
}

/**
 * Up button click down handler (reset)
 */
static void up_long_down_handler(ClickRecognizerRef recognizer, void *context) {
	// Begin again
	reset_sleep_period();
}

/**
 * Up button click up handler (reset)
 */
static void up_long_up_handler(ClickRecognizerRef recognizer, void *context) {
	// Take no action
}

/**
 * Back button single click handler
 */
static void back_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	show_keyboard();
}

/*
 * Single click handler on down button
 */
static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	if (!snooze_alarm())
		show_keyboard();
}

/*
 * Single click handler on select button
 */
static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	show_keyboard();
}

/*
 * Single click handler on up button
 */
static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	if (!cancel_alarm())
		show_keyboard();
}

/*
 * Button config
 */
void click_config_provider(Window *window) {
	const uint16_t delay_ms = 1500;
	window_long_click_subscribe(BUTTON_ID_SELECT, delay_ms, select_long_down_handler, select_long_up_handler);
	window_long_click_subscribe(BUTTON_ID_UP, delay_ms, up_long_down_handler, up_long_up_handler);
	window_single_click_subscribe(BUTTON_ID_BACK, back_single_click_handler);
	window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
	window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);
}
