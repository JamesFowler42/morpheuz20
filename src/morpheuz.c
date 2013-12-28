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

uint16_t two_minute_biggest = 0;
uint8_t sample_sets = 0;
uint8_t alarm_count;
time_t last_accel;

static int32_t last_from = -1;
static int32_t last_to = -1;

static uint32_t inbound_size;
static uint32_t outbound_size;

/*
 * Fire alarm
 */
static void fire_alarm() {
	alarm_count = 0;
	reset_tick_service(true);
}


/*
 * Do the alarm if needed
 */
void do_alarm() {

	// Alarm handling
	if (alarm_count >= ALARM_MAX) {
		reset_tick_service(false);
		return;
	}
	vibes_long_pulse();
	alarm_count++;
}

/*
 * Set the on-screen status text
 */
static void set_smart_status(int32_t sa_from, int32_t sa_to) {
	static char status_text[15];
	bool sa_smart = (sa_from != -1 && sa_to != -1);
	if (sa_smart) {
	  snprintf(status_text, sizeof(status_text), "%02ld:%02ld - %02ld:%02ld", sa_from >> 8, sa_from & 0xff, sa_to >> 8, sa_to & 0xff);
	} else {
	  strncpy(status_text, "", sizeof(status_text));
	}
	set_smart_status_on_screen(sa_smart, status_text);
}

/*
 * Incoming message dropped handler
 */
static void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Dropped: %d", reason);
}

/*
 * Outgoing message failed handler
 */
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Failed to Send: %d", reason);
}

/*
 * Incoming message handler
 */
static void in_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *ctrl_tuple = dict_find(iter, CTRL);
  Tuple *from_tuple = dict_find(iter, FROM);
  Tuple *to_tuple = dict_find(iter, TO);

  if (ctrl_tuple) {
	  int32_t ctrl_value = ctrl_tuple->value->int32;
	  if (ctrl_value & CTRL_ALARM) {
		  fire_alarm();
	  } 
	  if (ctrl_value & CTRL_INVERSE) {
		  invert_screen(true);
	  }
	  if (ctrl_value & CTRL_NORMAL) {
		  invert_screen(false);
	  }
	  APP_LOG(APP_LOG_LEVEL_DEBUG, "Ctrl received");
  }
  if (from_tuple) {
  	  last_from = from_tuple->value->int32;
	  set_smart_status(last_from, last_to);
	  APP_LOG(APP_LOG_LEVEL_DEBUG, "From received");
  }
  if (to_tuple) {
  	  last_to = to_tuple->value->int32;
	  set_smart_status(last_from, last_to);
	  APP_LOG(APP_LOG_LEVEL_DEBUG, "To received");
  }
}

/*
 * Send a message to javascript
 */
static void send_cmd(uint16_t biggest) {
	
  Tuplet biggest_tuple = TupletInteger(BIGGEST, biggest);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &biggest_tuple);
  dict_write_end(iter);

  app_message_outbox_send();
}

/*
 * Accurate buffer size count
 */
static void in_out_size_calc() {

  Tuplet out_values[] = {
        TupletInteger(BIGGEST, 0)
  };
        
  outbound_size = dict_calc_buffer_size_from_tuplets(out_values, ARRAY_LENGTH(out_values)) + FUDGE;
	
  Tuplet in_values[] = {
        TupletInteger(CTRL, 0),
        TupletInteger(FROM, 0),
        TupletInteger(TO, 0)
  };
        
  inbound_size = dict_calc_buffer_size_from_tuplets(in_values, ARRAY_LENGTH(in_values)) + FUDGE;
	
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Inbound buffer: %ld, Outbound buffer: %ld", inbound_size, outbound_size);
}
	
/*
 * Store our samples from each group until we have two minute's worth
 */
static void store_sample(uint16_t biggest) {
	if (biggest > two_minute_biggest)
		two_minute_biggest = biggest;
	sample_sets++;
	if (sample_sets > SAMPLES_IN_TWO_MINUTES) {
		send_cmd(two_minute_biggest);
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
	  accel_data_service_unsubscribe();
	  accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
	  accel_data_service_subscribe(25, &accel_data_handler);
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
void init_morpheuz() {
	
	  alarm_count = ALARM_MAX;
	
      last_accel = time(NULL);
	
	  // Register message handlers
  	  app_message_register_inbox_received(in_received_handler);
  	  app_message_register_inbox_dropped(in_dropped_handler);
  	  app_message_register_outbox_failed(out_failed_handler);
  
	  // Size calc
	  in_out_size_calc();
	
	  // Open buffers
	  app_message_open(inbound_size, outbound_size);

	  // Accelerometer
	  accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
	  accel_data_service_subscribe(25, &accel_data_handler);

}

/*
 * Close down accelerometer
 */
void deinit_morpheuz() {
	accel_data_service_unsubscribe();
	app_focus_service_unsubscribe();
}
