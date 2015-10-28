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

static InternalData internal_data;
static ConfigData config_data;
static bool save_config_requested = false;
static int32_t internal_data_checksum;
static bool no_record_warning = true;
static uint8_t last_progress_highest_entry = 254;

static int32_t previous_to_phone = DUMMY_PREVIOUS_TO_PHONE;

static bool version_sent = false;
static bool complete_outstanding = false;
static int8_t new_last_sent;
static time_t last_request;
static time_t last_response;

static void transmit_next_data(void *data);
static void reset_sleep_period_action(void *data);
static bool at_limit(int32_t offset);
static int32_t calc_offset();

extern AppTimer *auto_shutdown_timer; 

/*
 * Send a message to javascript
 */
static void send_to_phone(const uint32_t key, int32_t tophone) {

  Tuplet tuplet = TupletInteger(key, tophone);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    LOG_WARN("no outbox");
    return;
  }

  dict_write_tuplet(iter, &tuplet);
  dict_write_end(iter);

  app_message_outbox_send();

  last_request = time(NULL);

}

/*
 * Send a message to javascript
 */
static void send_point(uint8_t point, uint16_t biggest, bool ignore) {
  int32_t to_phone = join_value(point, (ignore ? 5000 : biggest));
  if (to_phone == previous_to_phone) {
    LOG_DEBUG("skipping send - data the same");
    app_timer_register(SHORT_RETRY_MS, transmit_next_data, NULL); // this is what would happen if we sent
    return;
  }
  previous_to_phone = to_phone;
  send_to_phone(KEY_POINT, to_phone);
}

/*
 * Incoming message handler
 */
static void in_received_handler(DictionaryIterator *iter, void *context) {

  // Got a ctrl message
  Tuple *ctrl_tuple = dict_find(iter, KEY_CTRL);
  if (ctrl_tuple) {
    
    int32_t ctrl_value = ctrl_tuple->value->int32;

    // If transmit is done then mark it
    if (ctrl_value & CTRL_TRANSMIT_DONE) {
      internal_data.transmit_sent = true;
      set_icon(true, IS_EXPORT);
      // If we're waiting for previous nights data to be sent, it now has been, reset and go
      if (complete_outstanding) {
        app_timer_register(COMPLETE_OUTSTANDING_MS, reset_sleep_period_action, NULL);
      }
      // If Morpheuz has woken to send data, then once the data is sent, speed up the shutdown
      // Normally around for 5 minutes but no need for that once data has been sent
      if (auto_shutdown_timer != NULL) {
        app_timer_reschedule(auto_shutdown_timer, TEN_SECONDS_MS);
      }
    }

    // If version is done then mark it
    if (ctrl_value & CTRL_VERSION_DONE) {
      version_sent = true;
      config_data.lazarus = ctrl_value & CTRL_LAZARUS;
      trigger_config_save();
    }

    // If gone off is done then mark that
    if (ctrl_value & CTRL_GONEOFF_DONE) {
      internal_data.gone_off_sent = true;
    }

    // Only let the last sent become it's new value after confirmation
    // from the JS
    if (ctrl_value & CTRL_SET_LAST_SENT) {
      LOG_DEBUG("in_received_handler - CTRL_SET_LAST_SENT to %d", new_last_sent);
      internal_data.last_sent = new_last_sent;
    }

    // If the request is to continue then do so.
    if (ctrl_value & CTRL_DO_NEXT) {
      app_timer_register(SHORT_RETRY_MS, transmit_next_data, NULL);
    }

    // Yes - must have comms
    set_icon(true, IS_COMMS);
    last_response = time(NULL);

  } 
}

/*
 * Send the version on initialisation
 */
static void send_version(void *data) {
  if (!version_sent) {
     if (bluetooth_connection_service_peek()) {
       app_timer_register(VERSION_SEND_INTERVAL_MS, send_version, NULL);
       send_to_phone(KEY_VERSION, VERSION);
     } else {
       app_timer_register(VERSION_SEND_SLOW_INTERVAL_MS, send_version, NULL);
     }
  }

}

/*
 * Calcuate buffer sizes and open comms
 */
EXTFN void open_comms() {

  LOG_DEBUG("internal_data %d, config_data %d", sizeof(internal_data), sizeof(config_data));

  // Register message handlers
  app_message_register_inbox_received(in_received_handler);

  // Incoming size
  Tuplet in_values[] = { TupletInteger(KEY_CTRL, 0) };

  uint32_t inbound_size = dict_calc_buffer_size_from_tuplets(in_values, ARRAY_LENGTH(in_values)) + FUDGE;

  LOG_DEBUG("I(%ld) O(%ld)", inbound_size, inbound_size);

  // Open buffers
  app_message_open(inbound_size, inbound_size);

  // Tell JS our version and keep trying until a reply happens
  app_timer_register(VERSION_SEND_INTERVAL_MS, send_version, NULL);
}

/*
 * Save the internal data structure
 */
EXTFN void save_internal_data() {
  int32_t checksum = dirty_checksum(&internal_data, sizeof(internal_data));
  if (checksum != internal_data_checksum) {
    LOG_DEBUG("save_internal_data (%d)", sizeof(internal_data));
    int written = persist_write_data(PERSIST_MEMORY_KEY, &internal_data, sizeof(internal_data));
    if (written != sizeof(internal_data)) {
      LOG_ERROR("save_internal_data error (%d)", written);
    } else {
      internal_data_checksum = checksum;
    }
  } else {
    LOG_DEBUG("save_internal_data no change");
  }
}

/*
 * Save internal data based on a timer (every few minutes)
 */
static void save_internal_data_timer(void *data) {
  save_internal_data();
  app_timer_register(PERSIST_MEMORY_MS, save_internal_data_timer, NULL);
}

/*
 * Wipe the internal data structures
 */
static void clear_internal_data() {
  memset(&internal_data, 0, sizeof(internal_data));
  internal_data_checksum = 0;
  internal_data.internal_ver = INTERNAL_VER;
}

/*
 * Set the progress marker
 */
static void set_progress_based_on_persist() {
  if (internal_data.highest_entry != last_progress_highest_entry) {
    set_progress();
    analogue_set_progress(internal_data.highest_entry + 1);
    last_progress_highest_entry = internal_data.highest_entry;
  }
}

/*
 * Read the internal data (or create it if missing)
 */
EXTFN void read_internal_data() {
  int read = persist_read_data(PERSIST_MEMORY_KEY, &internal_data, sizeof(internal_data));
  if (read != sizeof(internal_data) || internal_data.internal_ver != INTERNAL_VER) {
    clear_internal_data();
  } else {
    internal_data_checksum = dirty_checksum(&internal_data, sizeof(internal_data));
  }
  analogue_set_base(internal_data.base);
  set_progress_based_on_persist();
  app_timer_register(PERSIST_MEMORY_MS, save_internal_data_timer, NULL);
}

/*
 * Provide internal data structure to other units
 */
EXTFN InternalData *get_internal_data() {
  return &internal_data;
}

/*
 * Save the config data structure
 */
EXTFN void save_config_data(void *data) {
  LOG_DEBUG("save_config_data (%d)", sizeof(config_data));
  int written = persist_write_data(PERSIST_CONFIG_KEY, &config_data, sizeof(config_data));
  if (written != sizeof(config_data)) {
    LOG_ERROR("save_config_data error (%d)", written);
  }
  save_config_requested = false;
}

/*
 * Clear config if needed
 */
static void clear_config_data() {
  memset(&config_data, 0, sizeof(config_data));
  config_data.fromhr = FROM_HR_DEF;
  config_data.frommin = FROM_MIN_DEF;
  config_data.tohr = TO_HR_DEF;
  config_data.tomin = TO_MIN_DEF;
  config_data.from = to_mins(FROM_HR_DEF,FROM_MIN_DEF);
  config_data.to = to_mins(TO_HR_DEF,TO_MIN_DEF);
  config_data.lazarus = true;
  config_data.config_ver = CONFIG_VER;
}

/*
 * Read the config data (or create it if missing)
 */
EXTFN void read_config_data() {
  int read = persist_read_data(PERSIST_CONFIG_KEY, &config_data, sizeof(config_data));
  if (read != sizeof(config_data) || config_data.config_ver != CONFIG_VER) {
    clear_config_data();
  }
}

/*
 * Provide config data structure to other units
 */
EXTFN ConfigData *get_config_data() {
  return &config_data;
}

/*
 * Get the config to save at some point in the future
 */
EXTFN void trigger_config_save() {
  if (!save_config_requested) {
    app_timer_register(PERSIST_CONFIG_MS, save_config_data, NULL);
    save_config_requested = true;
  }
}

/*
 * Perform the actual reset action
 */
static void reset_sleep_period_action(void *data) {
  complete_outstanding = false;
  previous_to_phone = DUMMY_PREVIOUS_TO_PHONE;
  clear_internal_data();
  time_t now = time(NULL);
  internal_data.base = now;
  internal_data.last_sent = LAST_SENT_INIT;
  internal_data.has_been_reset = true;
  set_icon(true, IS_RECORD);
  set_icon(false, IS_IGNORE);
  set_icon(false, IS_EXPORT);
  analogue_set_base(internal_data.base);
  set_progress_based_on_persist();
  set_next_wakeup();
  if (config_data.smart) {
    show_notice(RESOURCE_ID_NOTICE_TIMER_RESET_ALARM);
    vibes_double_pulse();
  } else {
    show_notice(RESOURCE_ID_NOTICE_TIMER_RESET_NOALARM);
    vibes_short_pulse();
  } 
}

/*
 * Perform reset - before bed action
 */
EXTFN void reset_sleep_period() {
  
  // If we haven't transmitted data, we've actually been reset, we're at the limit and this hasn't been requested once before (allows a force reset)
  // Then show a notice, wait for completion and then we complete do the reset
  if (!internal_data.transmit_sent && internal_data.has_been_reset && !complete_outstanding && at_limit(calc_offset())) {
    show_notice(RESOURCE_ID_NOTICE_OUTSTANDING);
    complete_outstanding = true;
    return;
  }
  
  reset_sleep_period_action(NULL);
} 

/*
 * Force data resend
 */
EXTFN void resend_all_data(bool invoked_by_change_of_time) {
  if (!invoked_by_change_of_time) {
    show_notice(RESOURCE_ID_NOTICE_DATA_WILL_BE_RESENT_SHORTLY);
  } else if (internal_data.transmit_sent) {
    return; // If we've already sent the end marker then don't send again on change of times, we'll do this on next reset.
  }
  internal_data.last_sent = LAST_SENT_INIT;
  internal_data.gone_off_sent = false;
  internal_data.transmit_sent = false;
  set_icon(false, IS_EXPORT);
  previous_to_phone = DUMMY_PREVIOUS_TO_PHONE;
}

/*
 * Compute offset
 */
static int32_t calc_offset() {
  time_t now = time(NULL);

  return (now - internal_data.base) / DIVISOR;
}

/*
 * Are we at the limit for recording
 */
static bool at_limit(int32_t offset) {
  return (offset >= LIMIT || offset < 0);
}

/*
 * Set ignore on current time segment
 */
EXTFN void set_ignore_on_current_time_segment() {
  
  int32_t offset = calc_offset();
  
  if (at_limit(offset)) {
    set_icon(false, IS_IGNORE);
    return;
  }

  internal_data.ignore[offset] = !internal_data.ignore[offset];
  set_icon(internal_data.ignore[offset], IS_IGNORE);

}

/*
 * Store data returned from the accelerometer
 */
static void store_point_info(uint16_t point) {

  int32_t offset = calc_offset();

  if (at_limit(offset)) {
    set_icon(false, IS_RECORD);
    set_icon(false, IS_IGNORE);
    if (no_record_warning && !complete_outstanding) {
      show_notice(RESOURCE_ID_NOTICE_END_OF_RECORDING);
      no_record_warning = false;
    }
    return;
  }

  set_icon(true, IS_RECORD);
  set_icon(internal_data.ignore[offset], IS_IGNORE);

  // Remember the highest entry
  internal_data.highest_entry = offset;

  // Now store entries
  if (point > internal_data.points[offset])
    internal_data.points[offset] = point;

  // Show the progress bar
  set_progress_based_on_persist();
}

/*
 * Perform smart alarm function
 */
static bool smart_alarm(uint16_t point) {

  uint32_t now;
  uint32_t before;
  uint32_t after;

  // Are we doing smart alarm thing
  if (!config_data.smart)
    return false;

  // Now has the alarm been sounded yet
  if (internal_data.gone_off != 0)
    return false;

  // Are we in the right timeframe
  time_t timeNow = time(NULL);
  struct tm *time = localtime(&timeNow);
  now = to_mins(time->tm_hour, time->tm_min);

  if (now >= config_data.from && now < config_data.to) {

    // Work out the average
    int32_t total = 0;
    int32_t novals = 0;
    for (uint8_t i = 0; i <= internal_data.highest_entry; i++) {
      if (!internal_data.ignore[i]) {
        novals++;
        total += internal_data.points[i];
      }
    }
    if (novals == 0)
      novals = 1;
    int32_t threshold = total / novals;

    // Has the current point exceeded the threshold value
    if (point > threshold) {
      internal_data.gone_off = now;
      return true;
    } else {
      return false;
    }
  }

  before = now - 1;
  after = now + 1;

  // Or failing that have we hit the last minute we can
  if (now == config_data.to || before == config_data.to || after == config_data.to) {
    internal_data.gone_off = now;
    return true;
  }

  // None of the above
  return false;
}

static void transmit_points_or_background_data(int8_t last_sent) {

  LOG_DEBUG("transmit_points_or_background_data %d", last_sent);

  // Otherwise service as usual
  switch (last_sent) {
    case -4:
      send_to_phone(KEY_AUTO_RESET, config_data.auto_reset ? 1 : 0);
      break;
    case -3:
      send_to_phone(KEY_FROM, config_data.smart ? (int32_t) config_data.from : -1);
      break;
    case -2:
      send_to_phone(KEY_TO, config_data.smart ? (int32_t) config_data.to : -1);
      break;
    case -1:
      send_to_phone(KEY_BASE, internal_data.base);
      break;
    default:
      send_point(last_sent, internal_data.points[last_sent], internal_data.ignore[last_sent]);
      break;
  }
  new_last_sent = last_sent;
}

/*
 * Send data to phone
 */
static void transmit_data() {
  
  // Retry will occur on the next minute, so no connection, no sweat
  // Also don't bother if initial state or we haven't done the version handshake yet
  if (!version_sent || !internal_data.has_been_reset || !bluetooth_connection_service_peek()) {
    previous_to_phone = DUMMY_PREVIOUS_TO_PHONE;
    return;
  }
  
  #ifdef VOICE_SUPPORTED
  if (is_voice_system_active()) {
    previous_to_phone = DUMMY_PREVIOUS_TO_PHONE;
    return;
  }
  #endif

  // No comms if the last request went unanswered (out failed handler doesn't seem to spot too much)
  if (last_request > last_response) {
    set_icon(false, IS_COMMS);
  }

  // Send either base, from, to (if last sent is -1) or a point
  transmit_points_or_background_data(internal_data.last_sent);
}

/*
 * Send catchup data
 */
static void transmit_next_data(void *data) {

  // Retry will occur when we get a data sample set again
  // No need for timer here
  if (!bluetooth_connection_service_peek())
    return;
  
  #ifdef VOICE_SUPPORTED
  if (is_voice_system_active()) {
    return;
  }
  #endif

  // Have we already caught up - if so then finish with the gone off time, if present
  if (internal_data.last_sent >= internal_data.highest_entry) {
    if (internal_data.gone_off > 0 && !internal_data.gone_off_sent) {
      send_to_phone(KEY_GONEOFF, internal_data.gone_off);
    } else if (!internal_data.transmit_sent  && internal_data.has_been_reset && at_limit(calc_offset())) {
      send_to_phone(KEY_TRANSMIT, 0);
    }
    return;
  }

  // Transmit next load of data
  LOG_DEBUG("transmit_next_data %d<%d", internal_data.last_sent, internal_data.highest_entry);
  transmit_points_or_background_data(internal_data.last_sent + 1);
}

/*
 * Storage of points, raising of smart alarm and transmission to phone
 */
EXTFN void server_processing(uint16_t biggest) {
  if (!internal_data.has_been_reset) {
    if (no_record_warning) {
      show_notice(RESOURCE_ID_NOTICE_RESET_TO_START_USING);
      no_record_warning = false;
    }
  }
  store_point_info(biggest);
  if (smart_alarm(biggest)) {
    fire_alarm();
  }
  transmit_data();
}
