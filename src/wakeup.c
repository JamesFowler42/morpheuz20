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

/*
 * Clear next wakeup
 */
void clear_next_wakeup() {
  wakeup_cancel_all();
}

/*
 * Set the new wakeup, clear all others
 */
void set_next_wakeup() {

  // Reboot time is always 24 hours from last reboot
  time_t timestamp = (time_t) get_internal_data()->base + TWENTY_FOUR_HOURS_IN_SECONDS;

  // Make sure that if Morpheuz was last used days ago the restart time is not
  // in the past
  time_t now = time(NULL);
  while (timestamp < now) {
    timestamp += TWENTY_FOUR_HOURS_IN_SECONDS;
  }

  // Schedule (ensure we don't clash with another app)
  WakeupId id = E_RANGE;
  uint8_t count = 0;
  while (id == E_RANGE && count < 10) {
    id = wakeup_schedule(timestamp, WAKEUP_AUTO_RESTART, true);
    if (id == E_RANGE)
      timestamp += ONE_MINUTE;
    count++;
  }

  // Remember when auto is set for
  struct tm *time = localtime(&timestamp);
  get_config_data()->autohr = time->tm_hour;
  get_config_data()->automin = time->tm_min;

}

/*
 * This is where we go if Morpheuz is alive when the wakeup occurs
 */
static void wakeup_handler(WakeupId wakeup_id, int32_t cookie) {
  if (cookie == WAKEUP_AUTO_RESTART) {
    reset_sleep_period();
  }
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
      reset_sleep_period();
    }
  }
}

/*
 * Turn auto wakeup on or off
 */
void wakeup_toggle() {
  if (get_config_data()->auto_reset) {
    get_config_data()->auto_reset = false;
    clear_next_wakeup();
  } else if (get_internal_data()->has_been_reset) {
    get_config_data()->auto_reset = true;
    clear_next_wakeup();
    set_next_wakeup();
  }
}
