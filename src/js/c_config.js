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

/*exported mConst, mLang */

/*
 * Constants
 */
function mConst() {
  return {
    limit : 60,
    divisor : 600000,
    url : "http://ui.morpheuz.net/keith.j.fowler/morpheuz/view-",
    versionDef : "22",
    smartDef : "N",
    fromhrDef : "6",
    fromminDef : "30",
    tohrDef : "7",
    tominDef : "15",
    pushoverAPI : "https://api.pushover.net/1/messages.json",
    smartwatchProAPI : "http://2hk.smartwatch.pro/?source=Morpheuz&starts=",
    sampleIntervalMins : 10,
    awakeAbove : 1000,
    ctrlTransmitDone : 1,
    ctrlVersionDone : 2,
    ctrlGoneOffDone : 4,
    ctrlDoNext : 8,
    ctrlSetLastSent : 16,
    displayDateFmt : "WWW, NNN dd, yyyy hh:mm",
    swpUrlDate : "yyyy-MM-ddThh:mm:00",
    timeout : 4000,
    usageUrl : "http://ui.morpheuz.net/keith.j.fowler/morpheuz/usage/",
    usageSx : ".html",
    quotesUrl : "http://ui.morpheuz.net/keith.j.fowler/morpheuz/quotes.json"
  };
}

function mLang() {
  return {
    ok : "OK",
    sending : "Sending",
    disabled : "Disabled",
    cnc : "Could not calculate",
    sa : "Smart Alarm",
    startM : "Start Morpheuz",
    bedTime : "Bed time",
    bedNow : "Bed Now",
    cancelBed : "Cancel bed time"
  };
}