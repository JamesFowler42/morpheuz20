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
#include "analogue.h"

static uint16_t biggest_movement_in_one_minute = 0;

static int32_t previous_to_phone = 0;

static char version_context[] = "V";
static char goneoff_context[] = "G";
static char clear_context[] = "C";
static char do_next_context[] = "N";

static bool version_sent = false;

/*
 * Send a message to javascript
 */
static void send_to_phone(const uint32_t key, void *context, int32_t tophone) {

  Tuplet tuplet = TupletInteger(key, tophone);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  app_message_set_context(context);

  dict_write_tuplet(iter, &tuplet);
  dict_write_end(iter);

  app_message_outbox_send();

}

/*
 * Set the on-screen status text
 */
void set_smart_status() {
  static char status_text[15];
  snprintf(status_text, sizeof(status_text), "%d:%02d - %d:%02d", twenty_four_to_twelve(get_config_data()->fromhr), get_config_data()->frommin, twenty_four_to_twelve(get_config_data()->tohr), get_config_data()->tomin);
  set_weekend_icon(get_config_data()->smart && get_config_data()->weekend_until != 0);
  set_smart_status_on_screen(get_config_data()->smart, status_text);
  analogue_set_smart_times();
}

/*
 * Outgoing message failed handler
 */
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Send Fail %d", reason);
  show_comms_state(false);
  app_message_set_context(clear_context);
}

/*
 * Outgoing message success handler
 */
static void out_sent_handler(DictionaryIterator *iterator, void *context) {
  if (context == do_next_context) {
    app_timer_register(SHORT_RETRY_MS, transmit_next_data, NULL);
  } else if (context == goneoff_context) {
    get_internal_data()->gone_off_sent = true;
  } else if (context == version_context) {
    version_sent = true;
  }
  app_message_set_context(clear_context);
  show_comms_state(true);
}

/*
 * Has the version message been sent
 */
bool has_version_been_sent() {
  return version_sent;
}

/*
 * Incoming message handler
 */
static void in_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *ctrl_tuple = dict_find(iter, KEY_CTRL);
  if (ctrl_tuple) {
    show_comms_state(true);
    app_timer_register(SHORT_RETRY_MS, send_version, NULL);
  }
}

/*
 * Send a message to javascript
 */
void send_point(uint8_t point, uint16_t biggest, bool ignore) {
  int32_t to_phone = join_value(point, (ignore ? 5000 : biggest));
  if (to_phone == previous_to_phone) {
    app_timer_register(SHORT_RETRY_MS, transmit_next_data, NULL); // this is what would happen if we sent
    return;
  }
  previous_to_phone = to_phone;
  send_to_phone(KEY_POINT, do_next_context, to_phone);
}

/*
 * Send a version to javascript (called via timer)
 */
void send_version(void *data) {
  send_to_phone(KEY_VERSION, version_context, VERSION);
}

/*
 * Send a base to javascript
 */
void send_base() {
  send_to_phone(KEY_BASE, do_next_context, get_internal_data()->base);
}

/*
 * Send a from to javascript
 */
void send_from() {
  send_to_phone(KEY_FROM, do_next_context, get_config_data()->smart ? (int32_t) get_config_data()->from : -1);
}

/*
 * Send a to to javascript
 */
void send_to() {
  send_to_phone(KEY_TO, do_next_context, get_config_data()->smart ? (int32_t) get_config_data()->to : -1);
}

/*
 * Send a gone off to javascript
 */
void send_goneoff() {
  send_to_phone(KEY_GONEOFF, goneoff_context, get_internal_data()->gone_off);
}

/*
 * Calcuate buffer sizes and open comms
 */
static void open_comms() {

  // Incoming size
  Tuplet in_values[] = { TupletInteger(KEY_CTRL, 0) };

  uint32_t inbound_size = dict_calc_buffer_size_from_tuplets(in_values, ARRAY_LENGTH(in_values)) + FUDGE;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "I(%ld) O(%ld)", inbound_size, inbound_size);

  // Open buffers
  app_message_open(inbound_size, inbound_size);

  // Clear context
  app_message_set_context(clear_context);
}

/*
 * Store our samples from each group until we have two minute's worth
 */
static void store_sample(uint16_t biggest) {
  revive_clock_on_movement(biggest);
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
uint16_t every_minute_processing() {
  uint16_t last_biggest = biggest_movement_in_one_minute;
  validate_weekend();
  power_nap_check(biggest_movement_in_one_minute);
  server_processing(biggest_movement_in_one_minute);
  biggest_movement_in_one_minute = 0;
  return last_biggest;
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
 * Process deviation for an access
 */
static void do_axis(int16_t val, uint16_t *biggest, uint32_t avg) {
  uint16_t val_scale = scale_accel(val);
  if (val_scale < avg)
    val_scale = avg - val_scale;
  else
    val_scale -= avg;
  if (val_scale > *biggest)
    *biggest = val_scale;
}

/*
 * Process accelerometer data
 */
static void accel_data_handler(AccelData *data, uint32_t num_samples) {

  // Average the data
  uint32_t avg_x = 0;
  uint32_t avg_y = 0;
  uint32_t avg_z = 0;
  AccelData *dx = data;
  for (uint32_t i = 0; i < num_samples; i++, dx++) {
    // If vibe went off then discount everything - we're only loosing a 2.5 second set of samples, better than an
    // unwanted spike
    if (dx->did_vibrate) {
      return;
    }
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
    do_axis(d->x, &biggest, avg_x);
    do_axis(d->y, &biggest, avg_y);
    do_axis(d->z, &biggest, avg_z);
  }

  store_sample(biggest);
}

/*
 * Initialise comms and accelerometer
 */
void init_morpheuz(Window *window) {

  init_alarm();

  // Register message handlers
  app_message_register_inbox_received(in_received_handler);
  app_message_register_outbox_failed(out_failed_handler);
  app_message_register_outbox_sent(out_sent_handler);

  // Open comms
  open_comms();

  // Accelerometer
  accel_data_service_subscribe(25, accel_data_handler);
  accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);

  // Set click provider
  window_set_click_config_provider(window, (ClickConfigProvider) click_config_provider);

  // Set the smart status
  set_smart_status();
}

/*
 * Toggle weekend mode
 */
void toggle_weekend_mode() {
  if (!get_config_data()->smart) {
    show_notice(RESOURCE_ID_NOTICE_NEED_SMART_ALARM);
    return;
  }
  // Toggle weekend
  if (get_config_data()->weekend_until > 0) {
    // Turn off weekend
    get_config_data()->weekend_until = 0;
    set_smart_status();
  } else {
    // Turn on weekend
    get_config_data()->weekend_until = time(NULL) + WEEKEND_PERIOD;
    set_smart_status();
  }
}

