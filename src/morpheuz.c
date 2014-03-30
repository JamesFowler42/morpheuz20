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

static uint16_t biggest_movement_in_one_minute = 0;

static time_t last_accel;

static int32_t last_from = -1;
static int32_t last_to = -1;
static bool last_invert = false;

static uint32_t inbound_size;
static uint32_t outbound_size;

static char version_context[] = "V";
static char base_context[] = "B";
static char goneoff_context[] = "G";
static char point_context[] = "P";
static char clear_context[] = "P";

/*
 * Combine two ints as a long
 */
static int32_t join_value(int16_t top, int16_t bottom) {
	int32_t top_as_32 = top;
	int32_t bottom_as_32 = bottom;
	return top_as_32 << 16 | bottom_as_32;
}

/*
 * Send a message to javascript
 */
static bool send_to_phone(const uint32_t key, void *context, int32_t tophone) {

	Tuplet tuplet = TupletInteger(key, tophone);

	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);

	if (iter == NULL) {
		return false;
	}

	app_message_set_context(context);

	dict_write_tuplet(iter, &tuplet);
	dict_write_end(iter);

	app_message_outbox_send();

	return true;
}

/*
 * Set the on-screen status text
 */
void set_smart_status() {
	static char status_text[15];
	if (get_config_data()->smart) {
		if (get_config_data()->weekend_until == 0) {
			snprintf(status_text, sizeof(status_text), "%02d:%02d - %02d:%02d", get_config_data()->fromhr, get_config_data()->frommin, get_config_data()->tohr, get_config_data()->tomin);
		} else {
			strncpy(status_text, WEEKEND_TEXT, sizeof(status_text));
		}
	} else {
		strncpy(status_text, "", sizeof(status_text));
	}
	set_smart_status_on_screen(get_config_data()->smart, status_text);
}

/*
 * Incoming message dropped handler
 */
static void in_dropped_handler(AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_ERROR, "App Msg Drop %d", reason);
}

/*
 * Outgoing message failed handler
 */
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_ERROR, "App Msg Send Fail %d", reason);
	show_comms_state(false);
	app_message_set_context(clear_context);
}

/*
 * Outgoing message success handler
 */
static void out_sent_handler(DictionaryIterator *iterator, void *context) {
	if (context == point_context || context == base_context) {
		app_timer_register(SHORT_RETRY_MS, transmit_next_data, NULL);
	} else if (context == goneoff_context) {
		get_internal_data()->gone_off_sent = true;
	}
	app_message_set_context(clear_context);
	show_comms_state(true);
}

/*
 * Incoming message handler
 */
static void in_received_handler(DictionaryIterator *iter, void *context) {
	Tuple *ctrl_tuple = dict_find(iter, KEY_CTRL);
	Tuple *from_tuple = dict_find(iter, KEY_FROM);
	Tuple *to_tuple = dict_find(iter, KEY_TO);

	if (from_tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Got from");
		last_from = from_tuple->value->int32;
		set_config_data(last_from, last_to, last_invert);
		set_smart_status();
	}
	if (to_tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Got to");
		last_to = to_tuple->value->int32;
		set_config_data(last_from, last_to, last_invert);
		set_smart_status();
	}
	if (ctrl_tuple) {
		show_comms_state(true);
		int32_t ctrl_value = ctrl_tuple->value->int32;
		if (ctrl_value & CTRL_RESET) {
			reset_sleep_period(); // Phone trigger reset
		}
		if (ctrl_value & CTRL_INVERSE) {
			last_invert = true;
			set_config_data(last_from, last_to, last_invert);
			invert_screen();
		}
		if (ctrl_value & CTRL_NORMAL) {
			last_invert = false;
			set_config_data(last_from, last_to, last_invert);
			invert_screen();
		}
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Got ctrl. Vers sent");
		app_timer_register(SHORT_RETRY_MS, send_version, NULL);
	}
}

/*
 * Send a message to javascript
 */
void send_point(uint8_t point, uint16_t biggest) {
	int32_t to_phone = join_value(point, biggest);
	send_to_phone(KEY_POINT, point_context, to_phone);
}

/*
 * Send a version to javascript (called via timer)
 */
void send_version(void *data) {

	// Retry after a long delay
	if (!bluetooth_connection_service_peek()) {
		app_timer_register(LONG_RETRY_MS, send_version, NULL);
		return;
	}

	// Send unless the dictionary is unavailable
	if (!send_to_phone(KEY_VERSION, version_context, VERSION)) {
		APP_LOG(APP_LOG_LEVEL_WARNING, "Comms busy send_version re-timed");
		app_timer_register(SHORT_RETRY_MS, send_version, NULL);
	}
}

/*
 * Send a base to javascript
 */
void send_base(uint32_t base) {
	send_to_phone(KEY_BASE, base_context, base);
}

/*
 * Send a gone off to javascript
 */
void send_goneoff() {
	send_to_phone(KEY_GONEOFF, goneoff_context, get_internal_data()->gone_off);
}

/*
 * Accurate buffer size count
 */
static void in_out_size_calc() {

	Tuplet out_values[] = {
			TupletInteger(KEY_POINT, 0)      // Only send one value at a time so this is right
	};

	outbound_size = dict_calc_buffer_size_from_tuplets(out_values, ARRAY_LENGTH(out_values)) + FUDGE;

	Tuplet in_values[] = {
			TupletInteger(KEY_CTRL, 0),
			TupletInteger(KEY_FROM, 0),
			TupletInteger(KEY_TO, 0)
	};

	inbound_size = dict_calc_buffer_size_from_tuplets(in_values, ARRAY_LENGTH(in_values)) + FUDGE;

	APP_LOG(APP_LOG_LEVEL_INFO, "In buff %ld, Out buff %ld", inbound_size, outbound_size);
}

/*
 * Store our samples from each group until we have two minute's worth
 */
static void store_sample(uint16_t biggest) {
	if (biggest > biggest_movement_in_one_minute)
		biggest_movement_in_one_minute = biggest;
}

/*
 * Reset the weekend time
 */
static void validate_weekend() {
	if (get_config_data()->weekend_until > 0 && get_config_data()->weekend_until < time(NULL)) {
		get_config_data()->weekend_until = 0;
		set_smart_status();
	}
}

/*
 * Do something with samples every minute
 */
void every_minute_processing(int min_no) {
	validate_weekend();
	power_nap_check(biggest_movement_in_one_minute);
	server_processing(biggest_movement_in_one_minute);
	biggest_movement_in_one_minute = 0;
}

/*
 * Can't be bothered to play with negative numbers
 */
static uint16_t scale_accel(int16_t val) {
	int16_t retval = 4000 + val;
	if (retval < 0)
		retval = 0;
	return retval;
}

/*
 * Process accelerometer data
 */
static void accel_data_handler(AccelData *data, uint32_t num_samples) {

	// Self monitor memory
	last_accel = time(NULL);

	// Average the data
	uint32_t avg_x = 0;
	uint32_t avg_y = 0;
	uint32_t avg_z = 0;
	AccelData *dx = data;
	for (uint32_t i = 0; i < num_samples; i++, dx++) {
		avg_x += scale_accel(dx->x);
		avg_y += scale_accel(dx->y);
		avg_z += scale_accel(dx->z);
	}
	avg_x /= num_samples;
	avg_y /= num_samples;
	avg_z /= num_samples;

	// Work out deviations
	uint16_t biggest = 0;
	AccelData *d = data;
	for (uint32_t i = 0; i < num_samples; i++, d++) {
		uint16_t x = scale_accel(d->x) ;
		uint16_t y = scale_accel(d->y) ;
		uint16_t z = scale_accel(d->z) ;

		if (x < avg_x)
			x = avg_x - x;
		else
			x -= avg_x;

		if (y < avg_y)
			y = avg_y - y;
		else
			y -= avg_y;

		if (z < avg_z)
			z = avg_z - z;
		else
			z -= avg_z;

		// Store the worst case for that period
		if (!(d->did_vibrate)) {
			if (x > biggest) biggest = x;
			if (y > biggest) biggest = y;
			if (z > biggest) biggest = z;
		}
	}

	store_sample(biggest);
}

/*
 *	Self monitoring - if the accelerometer tell the wearer so as they can do something
 */
void self_monitor() {

	// Get time
	time_t now = time(NULL);

	// Or the accelerometer function having been dead for a while
	if (last_accel < (now - DISTRESS_WAIT_SEC)) {
		show_fatal(FATAL_ACCEL_CRASH);
	}
}

/*
 * Initialise comms and accelerometer
 */
void init_morpheuz(Window *window) {

	init_alarm();

	last_accel = time(NULL);

	// Register message handlers
	app_message_register_inbox_received(in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_register_outbox_failed(out_failed_handler);
	app_message_register_outbox_sent(out_sent_handler);

	// Size calc
	in_out_size_calc();

	// Open buffers
	app_message_open(inbound_size, outbound_size);

	// Ready to go
	app_message_set_context(clear_context);

	// Accelerometer
	accel_data_service_subscribe(25, accel_data_handler);
	accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);

	// Set click provider
	window_set_click_config_provider(window, (ClickConfigProvider) click_config_provider);

	// Set the smart status
	set_smart_status();

}

/*
 * Close down accelerometer
 */
void deinit_morpheuz() {
	accel_data_service_unsubscribe();
}

/*
 * Toggle weekend mode
 */
void toggle_weekend_mode() {
	if (!get_config_data()->smart) {
		show_notice(NOTICE_NEED_SMART_ALARM, false);
		return;
	}
	// Toggle weekend
	if (get_config_data()->weekend_until > 0) {
		// Turn off weekend
		get_config_data()->weekend_until = 0;
		show_notice(NOTICE_STOPPED_WEEKEND, false);
		set_smart_status();
	} else {
		// Turn on weekend
		get_config_data()->weekend_until = time(NULL) + WEEKEND_PERIOD;
		show_notice(NOTICE_STARTED_WEEKEND, false);
		set_smart_status();
	}
}

