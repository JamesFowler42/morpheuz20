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

#define POWER_NAP_SETTLE_INDICATOR "--"
#define POWER_NAP_OFF_INDICATOR ""
#define POWER_NAP_SETTLE_TIME "Power nap"
#define POWER_NAP_RUNNING     "Power nap: %d"
#define WEEKEND_TEXT      "Weekend"
#define NOTICE_TIMER_RESET_ALARM "Sleep well!\nChart reset.\nAlarm set."
#define NOTICE_TIMER_RESET_NOALARM "Sleep well!\nChart reset.\nNO ALARM."
#define NOTICE_END_OF_RECORDING "End of recording. Reset to start again."
#define NOTICE_RESET_TO_START_USING "Reset to start recording."
#define NOTICE_NEED_SMART_ALARM "No Smart alarm: no weekend mode!"
#define NOTICE_DATA_WILL_BE_RESENT_SHORTLY "Data will be resent. Please wait."

#define MENU_IGNORE "Ignore"
#define MENU_IGNORE_DES  "...next 10 mins"
#define MENU_RESET "Reset"
#define MENU_RESET_DES "Start new sleep cycle"
#define MENU_POWER_NAP "Power nap"
#define MENU_POWER_NAP_DES "Refreshment"
#define MENU_WEEKEND "Weekend"
#define MENU_WEEKEND_DES "Lay-in mode"
#define MENU_RESEND "Resend"
#define MENU_RESEND_DES "Resend data to phone"
#define MENU_INVERSE "Inverse"
#define MENU_INVERSE_DES "Black on white?"
#define MENU_ANALOGUE "Analogue"
#define MENU_ANALOGUE_DES "Hands or classic?"
#define MENU_QUIT "Quit"
#define MENU_QUIT_DES "...or a long press on back"
#define MENU_SNOOZE "Snooze Alarm"
#define MENU_SNOOZE_DES "... or press down"
#define MENU_CANCEL "Cancel Alarm"
#define MENU_CANCEL_DES "... or press up"

#define COPYRIGHT "\n© 2014\nJames Fowler"

#define MORPHEUZ "Morpheuz"

#endif /* LANGUAGE_H_ */
