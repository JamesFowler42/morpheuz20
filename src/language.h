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

#ifndef LANGUAGE_H_
#define LANGUAGE_H_

#define POWER_NAP_SETTLE_TIME "Power nap"
#define POWER_NAP_RUNNING     "Power nap: %d"
#define WEEKEND_TEXT      "Weekend"
#define NOTICE_TIMER_RESET_ALARM "Sleep well!\nChart reset.\nAlarm set."
#define NOTICE_TIMER_RESET_NOALARM "Sleep well!\nChart reset.\nNO ALARM."
#define NOTICE_STARTED_POWER_NAP "Power nap started."
#define NOTICE_STOPPED_POWER_NAP "Power nap stopped."
#define NOTICE_TIME_TO_WAKE_UP "Time to wake up!"
#define NOTICE_ALARM_CANCELLED "Alarm cancelled."
#define NOTICE_END_OF_RECORDING "End of recording. Reset to start again."
#define NOTICE_RESET_TO_START_USING "Reset to start recording."
#define NOTICE_SNOOZE_ACTIVATED "Snooze 9 minutes."
#define NOTICE_STARTED_WEEKEND "Weekend mode.\nNo alarm for 12hrs."
#define NOTICE_STOPPED_WEEKEND "Weekend mode cancelled."
#define NOTICE_NEED_SMART_ALARM "No Smart alarm: no weekend mode!"
#define NOTICE_DATA_WILL_BE_RESENT_SHORTLY "Data will be resent. Please wait."
#define NOTICE_IGNORING "Ignoring this 10min period."
#define NOTICE_NOT_IGNORING "Not ignoring this 10min period."

#define FATAL_ACCEL_CRASH "\nAccelerometer API not returning data.\n\nHold back to leave Morpheuz.\nContact support from the Pebble app on your phone.\nReboot your Pebble."

#define COPYRIGHT "\n(c) 2014\nJames Fowler"

#define R1_TEXT "Reset (hold)\nStop Alarm"
#define R2_TEXT "Power nap (hold)\nIgnore (2x)"
#define R3_TEXT "Weekend (hold)\nSnooze"
#define L1_TEXT "Exit (hold)\nResend(2x)"

#define MORPHEUZ "Morpheuz"

#endif /* LANGUAGE_H_ */
