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

static uint16_t two_minute_biggest = 0;
static uint8_t sample_sets = 0;
static uint8_t alarm_count;
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
 * Fire alarm
 */
void fire_alarm() {
	alarm_count = 0;
	reset_tick_service(true);
	show_notice(NOTICE_TIME_TO_WAKE_UP);
}

/*
 * Do the alarm if needed
 */
void do_alarm() {

	// Already hit the limit
	if (alarm_count >= ALARM_MAX) {
		return;
	}

	// Vibrate
	vibes_long_pulse();
	alarm_count++;

	// Reset timers and powernap if needed
	if (alarm_count >= ALARM_MAX) {
		reset_tick_service(false);
		power_nap_reset();
	}
}

/*
 * Cancel alarm - if there is one
 */
void cancel_alarm() {

	// Already hit the limit
	if (alarm_count >= ALARM_MAX) {
		return;
	}

	alarm_count = ALARM_MAX - 1;

	show_notice(NOTICE_ALARM_CANCELLED);

}

/*
 * Set the on-screen status text
 */
void set_smart_status() {
	static char status_text[15];
	if (get_config_data()->smart) {
		snprintf(status_text, sizeof(status_text), "%02d:%02d - %02d:%02d", get_config_data()->fromhr, get_config_data()->frommin, get_config_data()->tohr, get_config_data()->tomin);
	} else {
		strncpy(status_text, "", sizeof(status_text));
	}
	set_smart_status_on_screen(get_config_data()->smart, status_text);
}

/*
 * Incoming message dropped handler
 */
static void in_dropped_handler(AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_ERROR, "App Message Dropped: %d", reason);
}

/*
 * Outgoing message failed handler
 */
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_ERROR, "App Message Failed to Send: %d", reason);
	show_comms_state(false);
	app_message_set_context(clear_context);
}

/*
 * Outgoing message success handler
 */
static void out_sent_handler(DictionaryIterator *iterator, void *context) {
	if (context == point_context || context == base_context) {
		app_timer_register(SHORT_RETRY_MS, transmit_next_data, NULL);
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
		APP_LOG(APP_LOG_LEVEL_DEBUG, "From received");
		last_from = from_tuple->value->int32;
		set_config_data(last_from, last_to, last_invert);
		set_smart_status();
	}
	if (to_tuple) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "To received");
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
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Ctrl received - version sent");
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
void send_goneoff(void *data) {
	// Retry after a long delay
	if (!bluetooth_connection_service_peek()) {
		app_timer_register(LONG_RETRY_MS, send_version, NULL);
		return;
	}

	if (!send_to_phone(KEY_GONEOFF, goneoff_context, get_internal_data()->gone_off)) {
		APP_LOG(APP_LOG_LEVEL_WARNING, "Comms busy send_base re-timed");
		app_timer_register(SHORT_RETRY_MS, send_goneoff, NULL);
	}
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

	APP_LOG(APP_LOG_LEVEL_INFO, "Inbound buffer: %ld, Outbound buffer: %ld", inbound_size, outbound_size);
}

/*
 * Store our samples from each group until we have two minute's worth
 */
static void store_sample(uint16_t biggest) {
	if (biggest > two_minute_biggest)
		two_minute_biggest = biggest;
	sample_sets++;
	if (sample_sets > SAMPLES_IN_TWO_MINUTES) {
		power_nap_check(two_minute_biggest);
		server_processing(two_minute_biggest);
		sample_sets = 0;
		two_minute_biggest = 0;
	}
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

	// Show it's working
	toggle_zzz();

	// Self monitor memory
	last_accel = time(NULL);

	// Average the data
	uint32_t avg_x = 0;
	uint32_t avg_y = 0;
	uint32_t avg_z = 0;
	AccelData *dx = data;
	for (uint32_t i = 0; i < num_samples; i++, dx++) {
		avg_x = avg_x + scale_accel(dx->x);
		avg_y = avg_y + scale_accel(dx->y);
		avg_z = avg_z + scale_accel(dx->z);
	}
	avg_x = avg_x / num_samples;
	avg_y = avg_y / num_samples;
	avg_z = avg_z / num_samples;

	// Work out deviations
	bool vibrated = false;
	uint16_t biggest = 0;
	AccelData *d = data;
	for (uint32_t i = 0; i < num_samples; i++, d++) {
		uint16_t x = scale_accel(d->x) ;
		uint16_t y = scale_accel(d->y) ;
		uint16_t z = scale_accel(d->z) ;

		if (x < avg_x)
			x = avg_x - x;
		else
			x = x - avg_x;

		if (y < avg_y)
			y = avg_y - y;
		else
			y = y - avg_y;

		if (z < avg_z)
			z = avg_z - z;
		else
			z = z - avg_z;

		// Store the largest deviation in the period
		if (x > biggest) biggest = x;
		if (y > biggest) biggest = y;
		if (z> biggest) biggest = z;

		// Did we have a bit of a vibration?
		if (d->did_vibrate)
			vibrated = true;
	}
	if (vibrated) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Discarded %d - vibration", biggest);
		store_sample(0); // Keep to timeframe but don't let large value disrupt
	} else {
		store_sample(biggest);
	}
}

/*
 * Reset the application to try and keep everything working
 */
static void reset() {
	APP_LOG(APP_LOG_LEVEL_ERROR, "Restarting accelerometer");
	accel_data_service_unsubscribe();
	accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
	accel_data_service_subscribe(25, accel_data_handler);
}

/*
 *	Self monitoring - if the accelerometer stop working reset the app so we can continue
 */
void self_monitor() {

	// Get time
	time_t now = time(NULL);

	// Or the accelerometer function having been dead for a while
	if (last_accel < (now - DISTRESS_WAIT_SEC)) {
		reset();
	}

}

/*
 * Initialise comms and accelerometer
 */
void init_morpheuz(Window *window) {

	alarm_count = ALARM_MAX;

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
	accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
	accel_data_service_subscribe(25, accel_data_handler);

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
