/*
 * Morpheuz Sleep Monitor
 *
 * Copyright (c) 2013-2014 James Fowler
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

/*
 * Save the internal data structure
 */
void save_internal_data() {
  int32_t checksum = dirty_checksum(&internal_data, sizeof(internal_data));
  if (checksum != internal_data_checksum) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "save_internal_data (%d)", sizeof(internal_data));
    int written = persist_write_data(PERSIST_MEMORY_KEY, &internal_data, sizeof(internal_data));
    if (written != sizeof(internal_data)) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "save_internal_data error (%d)", written);
    } else {
      internal_data_checksum = checksum;
    }
  } else {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "save_internal_data no change");
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
  internal_data.has_been_reset = false;
}

/*
 * Set the progress marker
 */
static void set_progress_based_on_persist() {
  if (internal_data.highest_entry != last_progress_highest_entry) {
    set_progress(internal_data.highest_entry + 1);
    analogue_set_progress(internal_data.highest_entry + 1);
    last_progress_highest_entry = internal_data.highest_entry;
  }
}

/*
 * Read the internal data (or create it if missing)
 */
void read_internal_data() {
  int read = persist_read_data(PERSIST_MEMORY_KEY, &internal_data, sizeof(internal_data));
  if (read != sizeof(internal_data)) {
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
InternalData *get_internal_data() {
  return &internal_data;
}

/*
 * Save the config data structure
 */
void save_config_data(void *data) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "save_config_data (%d)", sizeof(config_data));
  int written = persist_write_data(PERSIST_CONFIG_KEY, &config_data, sizeof(config_data));
  if (written != sizeof(config_data)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "save_config_data error (%d)", written);
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
}

/*
 * Read the config data (or create it if missing)
 */
void read_config_data() {
  int read = persist_read_data(PERSIST_CONFIG_KEY, &config_data, sizeof(config_data));
  if (read != sizeof(config_data)) {
    clear_config_data();
  }
}

/*
 * Provide config data structure to other units
 */
ConfigData *get_config_data() {
  return &config_data;
}

/*
 * Get the config to save at some point in the future
 */
void trigger_config_save() {
  if (!save_config_requested) {
    app_timer_register(PERSIST_CONFIG_MS, save_config_data, NULL);
    save_config_requested = true;
  }
}

/*
 * Perform reset - either from watch or phone
 */
void reset_sleep_period() {
  clear_internal_data();
  time_t now = time(NULL);
  internal_data.base = now;
  internal_data.last_sent = LAST_SENT_INIT;
  internal_data.has_been_reset = true;
  show_record(false);
  show_ignore_state(false);
  analogue_set_base(internal_data.base);
  set_progress_based_on_persist();
  if (get_config_data()->auto_reset) {
    clear_next_wakeup();
    set_next_wakeup();
  }
  if (config_data.smart && config_data.weekend_until == 0) {
    show_notice(RESOURCE_ID_NOTICE_TIMER_RESET_ALARM);
    vibes_double_pulse();
  } else {
    show_notice(RESOURCE_ID_NOTICE_TIMER_RESET_NOALARM);
    vibes_short_pulse();
  }

}

/*
 * Force data resend
 */
void resend_all_data(bool silent) {
  if (!silent) {
    show_notice(RESOURCE_ID_NOTICE_DATA_WILL_BE_RESENT_SHORTLY);
  }
  internal_data.last_sent = LAST_SENT_INIT;
  internal_data.gone_off_sent = false;
}

/*
 * Compute offset
 */
static int32_t calc_offset() {
  time_t now = time(NULL);

  return (now - internal_data.base) / DIVISOR;
}

/*
 * Set ignore on current time segment
 */
void set_ignore_on_current_time_segment() {

  int32_t offset = calc_offset();

  if (offset >= LIMIT || offset < 0) {
    show_ignore_state(false);
    return;
  }

  internal_data.ignore[offset] = !internal_data.ignore[offset];
  show_ignore_state(internal_data.ignore[offset]);

}

/*
 * Store data returned from the watch
 */
static void store_point_info(uint16_t point) {

  int32_t offset = calc_offset();

  if (offset >= LIMIT || offset < 0) {
    show_record(false);
    show_ignore_state(false);
    if (no_record_warning) {
      show_notice(RESOURCE_ID_NOTICE_END_OF_RECORDING);
      no_record_warning = false;
    }
    return;
  }

  show_record(true);
  show_ignore_state(internal_data.ignore[offset]);

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

  // Not alarming at the moment
  if (config_data.weekend_until > 0)
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

static void transmit_points_or_background_data() {
  switch (internal_data.last_sent) {
    case -3:
      send_from();
      break;
    case -2:
      send_to();
      break;
    case -1:
      send_base();
      break;
    default:
      send_point(internal_data.last_sent, internal_data.points[internal_data.last_sent], internal_data.ignore[internal_data.last_sent]);
      break;
  }
}

/*
 * Send data to phone
 */
static void transmit_data() {

  // Retry will occur when we get a data sample set again
  // No need for timer here
  if (!bluetooth_connection_service_peek())
    return;

  // Send either base, from, to (if last sent is -1) or a point
  transmit_points_or_background_data();
}

/*
 * Send catchup data
 */
void transmit_next_data(void *data) {

  // Retry will occur when we get a data sample set again
  // No need for timer here
  if (!bluetooth_connection_service_peek())
    return;

  // Have we already caught up - if so then finish with the gone off time, if present
  if (internal_data.last_sent >= internal_data.highest_entry) {
    if (internal_data.gone_off > 0 && !internal_data.gone_off_sent) {
      APP_LOG(APP_LOG_LEVEL_INFO, "send goneoff");
      send_goneoff();
    } else if (!has_version_been_sent()) {
      app_timer_register(SHORT_RETRY_MS, send_version, NULL);
    }
    return;
  }

  // Transmit next load of data
  APP_LOG(APP_LOG_LEVEL_INFO, "transmit_next_data %d<%d", internal_data.last_sent, internal_data.highest_entry);
  internal_data.last_sent++;
  transmit_points_or_background_data();
}

/*
 * Storage of points and raising of smart alarm
 */
void server_processing(uint16_t biggest) {
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
