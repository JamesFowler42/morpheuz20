/*
 * Morpheuz Sleep Monitor
 *
 * Copyright (c) 2013-2016 James Fowler
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

// Times (in seconds) between each buzz (gives a progressive alarm and gaps between phases)
static uint8_t alarm_pattern[] = { 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 60 };

#define ALARM_LIMIT (ARRAY_LENGTH(alarm_pattern) * ALARM_PATTERN_MAX_REPEAT)

static uint8_t alarm_count = ALARM_LIMIT;
static AppTimer *alarm_timer;

/*
 * Alarm timer loop
 */
static void do_alarm(void *data) {

  // Already hit the limit
  if (alarm_count >= ALARM_LIMIT) {
    return;
  }
  
  // Flash the light every 10th entry in the alarm pattern
  // Did this once - took it out for some random reason
  if (alarm_count%10 == 0) {
    light_enable_interaction();
  }

  // Don't vibe if waiting for voice
  if (!is_voice_system_active()) {
    // Vibrate
    if (alarm_count < 10)
      vibes_short_pulse();
    else
      vibes_long_pulse();
  }

  // Prepare the time for the next buzz (this gives progressing and phasing)
  alarm_timer = app_timer_register(((uint16_t) alarm_pattern[alarm_count % (ARRAY_LENGTH(alarm_pattern))]) * 1000, do_alarm, NULL);

  alarm_count++;

  // Reset powernap and finish alarm
  if (alarm_count >= ALARM_LIMIT) {
    power_nap_reset();
    show_alarm_visuals(false);
  }

}

/*
 * Fire alarm
 */
EXTFN void fire_alarm() {
  alarm_count = 0;
  do_alarm(NULL);
  show_alarm_visuals(true);
}

/*
 * Snooze alarm
 */
EXTFN void snooze_alarm() {
  // Already hit the limit so cannot snooze
  if (alarm_count >= ALARM_LIMIT) {
    return;
  }

  // Set alarm to go off in 9 minutes
  app_timer_reschedule(alarm_timer, SNOOZE_PERIOD_MS);

  // Reset alarm sequence
  alarm_count = 0;
}

/*
 * Cancel alarm - if there is one
 */
EXTFN void cancel_alarm() {

  // Already hit the limit - nothing to cancel
  if (alarm_count >= ALARM_LIMIT) {
    return;
  }

  // Stop the timer
  app_timer_cancel(alarm_timer);

  // Max out the count
  alarm_count = ALARM_LIMIT;

  // Reset power nap if not already done so
  power_nap_reset();

  // Clear the alarm indicator
  show_alarm_visuals(false);
}


