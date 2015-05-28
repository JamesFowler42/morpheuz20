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

/*
 * Build a wakeup entry
 */
static void build_wakeup_entry(time_t timestamp, int32_t cookie) {
  // Schedule (ensure we don't clash with another app)
  WakeupId id = E_RANGE;
  uint8_t count = 0;
  while (id == E_RANGE && count < 10) {
    id = wakeup_schedule(timestamp, cookie, false);
    if (id == E_RANGE)
      timestamp += ONE_MINUTE;
    count++;
  }
  if (id < 0) {
    LOG_ERROR("Wakeup for cookie=%ld rejected with %ld", cookie, id);
  } else {
    LOG_DEBUG("Wakeup for cookie=%ld ok", cookie);
  }
}

/*
 * Set the new wakeup, clear all others
 */
void set_next_wakeup() {

  // Clear wakeups
  wakeup_cancel_all();

  time_t now = time(NULL);

  // Auto wakeup if set
  if (get_config_data()->auto_reset) {

    // Reboot time is always 24 hours from last reboot
    time_t timestamp = (time_t) get_internal_data()->base + TWENTY_FOUR_HOURS_IN_SECONDS;

    // Make sure that if Morpheuz was last used days ago the restart time is not
    // in the past
    while (timestamp < now) {
      timestamp += TWENTY_FOUR_HOURS_IN_SECONDS;
    }

    build_wakeup_entry(timestamp, WAKEUP_AUTO_RESTART);

    // Remember when auto is set for
    struct tm *time = localtime(&timestamp);
    get_config_data()->autohr = time->tm_hour;
    get_config_data()->automin = time->tm_min;
  }

  // Wakeup for transmit
  time_t revive_timestamp = (time_t) get_internal_data()->base + ELEVEN_HOURS_IN_SECONDS;
  if (revive_timestamp > now) {
    build_wakeup_entry(revive_timestamp, WAKEUP_FOR_TRANSMIT);
  }
}

/*
 * This is where we go if Morpheuz is alive when the wakeup occurs
 */
static void wakeup_handler(WakeupId wakeup_id, int32_t cookie) {
  if (cookie == WAKEUP_AUTO_RESTART) {
    reset_sleep_period();
  }
}

/**
 * Close in five minutes
 */
static void close_morpheuz_timer(void *data) {
  close_morpheuz();
}

/*
 * Wakeup service initialisation hook
 */
void wakeup_init() {
  WakeupId wakeup_id;
  int32_t cookie;
  wakeup_service_subscribe(wakeup_handler);
  if (launch_reason() == APP_LAUNCH_WAKEUP) {
    wakeup_get_launch_event(&wakeup_id, &cookie);
    if (cookie == WAKEUP_AUTO_RESTART) {
      start_worker();
      reset_sleep_period();
    } else if (cookie == WAKEUP_FOR_TRANSMIT) {
      app_timer_register(FIVE_MINUTES_MS, close_morpheuz_timer, NULL);
    }
#ifdef PBL_PLATFORM_BASALT
  } else if (launch_reason() == APP_LAUNCH_TIMELINE_ACTION) {
    switch (launch_get_args()) {
      case TIMELINE_LAUNCH_USE:
      start_worker();
      break;
      case TIMELINE_LAUNCH_SLEEP_NOW:
      start_worker();
      reset_sleep_period();
      break;
      case TIMELINE_LAUNCH_CLEAR_AUTOSLEEP:
      get_config_data()->auto_reset = false;
      set_next_wakeup();
      app_timer_register(TEN_SECONDS_MS, close_morpheuz_timer, NULL);
      break;
    }
#endif
  } else {
    start_worker();
  }

}

/*
 * Turn auto wakeup on or off
 */
void wakeup_toggle() {
  get_config_data()->auto_reset = get_internal_data()->has_been_reset ? !get_config_data()->auto_reset : false;
  set_next_wakeup();
  resend_all_data(true); // Force resend - we've fiddled with the wakeup
}
