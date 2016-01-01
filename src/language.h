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

#ifndef LANGUAGE_H_
#define LANGUAGE_H_

#define POWER_NAP_SETTLE_INDICATOR "--"
#define POWER_NAP_OFF_INDICATOR ""

#define MENU_IGNORE "Ignore"
#define MENU_IGNORE_DES  "...next 10 mins"
#define MENU_RESET "Bed time"
#define MENU_RESET_DES "Start new sleep cycle"
#define MENU_POWER_NAP "Power nap"
#define MENU_POWER_NAP_DES "A 27 min nap"
#define MENU_RESEND "Resend"
#define MENU_RESEND_DES "Resend data to phone"
#define MENU_ANALOGUE "Analogue"
#define MENU_ANALOGUE_DES "Hands or classic?"
#define MENU_QUIT "Quit"
#define MENU_QUIT_DES "Shut down Morpheuz"
#define MENU_STOP_AND_QUIT "Stop & quit"
#define MENU_STOP_AND_QUIT_DES "Woke and got up"
#define MENU_SNOOZE "Snooze Alarm"
#define MENU_CANCEL "Cancel Alarm"
#define MENU_AUTO_RESET "Auto bed time"
#define MENU_AUTO_RESET_DES_OFF "... same time again"
#define MENU_AUTO_RESET_DES_ON "... at %d:%02d%s"
#define MENU_SMART_ALARM "Set Smart Alarm"
#define MENU_PRESET "Presets"
#define MENU_PRESET_DES "Alarm time memories"
#define MENU_TOGGLE_ALARM "Alarm on/off"
#define MENU_TOGGLE_ALARM_DES "Turn alarm on/off"
  
#define MENU_SMART_ALARM_TIME_FORMAT "%d:%02d%s - %d:%02d%s"
#define MENU_SMART_ALARM_END_TIME_FORMAT "%d:%02d%s"
#define TIME_RANGE_LEN 20
  
#define DATE_FORMAT "%a, %b %e"
#define DATE_FORMAT_LEN 12
  
#define MENU_SNOOZE_DES  "... or press up"
#define MENU_CANCEL_DES "... or press down"

#define TEXT_AM "AM"
#define TEXT_PM "PM"
#define TEXT_AM_PAD " AM"
#define TEXT_PM_PAD " PM"

#define EARLIEST "Earliest"
#define LATEST "Latest"

#define COPYRIGHT "\n© 2016\nJames Fowler"

#define MORPHEUZ "Morpheuz"

#endif /* LANGUAGE_H_ */
