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

/*global window, nvl, mLang, mConst, makeGetAjaxCall */
/*exported smartwatchProConfigured, smartwatchProTransmit, calculateStats, generateCopyLinkData, mThres */

/*
 * Is Smartwatch Pro configured?
 */
function smartwatchProConfigured() {
  var doSwp = nvl(window.localStorage.getItem("swpdo"), "N");
  return (doSwp === "Y");
}

/*
 * Transmit to smartwatch pro
 */
function smartwatchProTransmit() {
  try {
    var doSwp = nvl(window.localStorage.getItem("swpdo"), "N");
    if (doSwp !== "Y") {
      window.localStorage.setItem("swpstat", mLang().disabled);
      console.log("smartwatchProTransmit: swpdo not set");
      return;
    }
    window.localStorage.setItem("swpstat", mLang().sending);
    var stats = calculateStats();
    if (stats.tbegin === null || stats.tends === null) {
      window.localStorage.setItem("swpstat", mLang().cnc);
      console.log("smartwatchProTransmit: stats couldn't be calculated");
      return;
    }
    var token = Pebble.getAccountToken();
    var swpUrl = mConst().smartwatchProAPI + stats.tbegin.format(mConst().swpUrlDate) + "&ends=" + stats.tends.format(mConst().swpUrlDate) + "&at=" + token;
    console.log("smartwatchProTransmit: url=" + swpUrl);
    makeGetAjaxCall(swpUrl, function(resp) {
      console.log("smartwatchProTransmit: " + JSON.stringify(resp));
      if (resp.status !== 1) {
        window.localStorage.setItem("swpdo", "N"); // Turn off send on error
        window.localStorage.setItem("swpstat", JSON.stringify(resp.errors));
      } else {
        window.localStorage.setItem("swpstat", mLang().ok);
      }
    });
  } catch (err) {
    window.localStorage.setItem("swpdo", "N"); // Turn off send on error
    window.localStorage.setItem("swpstat", err.message);
  }
}

/*
 * Calculate stats
 */
function calculateStats() {
  var base = parseInt(window.localStorage.getItem("base"), 10);
  var goneoff = nvl(window.localStorage.getItem("goneOff"), "N");
  var splitup = extractSplitup();

  // Get the full set of data up to the wake up point.
  // Ignore nulls
  var timeStartPoint = new Date(base);
  var firstSleep = true;
  var tbegin = null;
  var tends = null;
  var tendsStop = null;
  var ibegin = null;
  var iends = null;
  var iendsStop = null;
  for (var i = 0; i < splitup.length; i++) {
    if (splitup[i] === "") {
      continue;
    }
    var data = parseInt(splitup[i], 10);
    var teststr1 = timeStartPoint.format("hhmm");
    var timeStartPoint1 = timeStartPoint;
    timeStartPoint = timeStartPoint.addMinutes(mConst().sampleIntervalMins);
    var teststr2 = timeStartPoint.format("hhmm");
    if (goneoff != "N" && goneoff >= teststr1 && goneoff <= teststr2) {
      tends = returnAbsoluteMatch(timeStartPoint1, timeStartPoint, goneoff);
      iends = i;
      break;
    } else if (data != -1 && data != -2 && data <= mThres().awakeAbove) {
      if (firstSleep) {
        tbegin = timeStartPoint1;
        ibegin = i;
        firstSleep = false;
      }
      tendsStop = timeStartPoint;
      iendsStop = i;
    }
  }

  // If we haven't got a regular end because of an alarm, then find
  // the last time they were below waking levels of movement
  if (tends === null && tendsStop !== null) {
    tends = tendsStop;
    iends = iendsStop;
  }
  
    // Compute the stats within the bounds of the start and stop times
  var awake = 0;
  var deep = 0;
  var light = 0;
  var ignore = 0;
  if (ibegin !== null && iends !== null) {
    for (var j = ibegin; j <= iends; j++) {
      if (splitup[j] === "") {
        continue;
      }
      var data2 = parseInt(splitup[j], 10);
      if (data2 == -1 || data2 == -2) {
        ignore++;
      } else if (data2 > mThres().awakeAbove) {
        awake++;
      } else if (data2 > mThres().lightAbove) {
        light++;
      } else {
        deep++;
      }
    }
  }

  return {
    "tbegin" : tbegin,
    "tends" : tends, 
    "deep" : deep,
    "light" : light,
    "awake" : awake,
    "ignore" : ignore
  };
}

/*
 * Locate match one minute at a time
 */
function returnAbsoluteMatch(early, late, actualstr) {
  var point = early;
  while (point.getTime() < late.getTime()) {
    var teststr = point.format("hhmm");
    if (actualstr === teststr) {
      return point;
    }
    point = point.addMinutes(1);
  }
  return early;
}

/*
 * Extract splitup array from local storage
 */
function extractSplitup() {
  var splitup = [];
  for (var j = 0; j < mConst().limit; j++) {
    var entry = "P" + j;
    var valueStr = window.localStorage.getItem(entry);
    if (valueStr === null) {
      splitup[j] = "-1";
    } else {
      splitup[j] = valueStr;
    }
  }
  return splitup;
}

/*
 * Prepare the data for the ifttt value3
 */
function generateCopyLinkData() {
  
  var lb = "<br/>";
  
  var base = parseInt(window.localStorage.getItem("base"), 10);
  var goneoff = nvl(window.localStorage.getItem("goneOff"), "N");
  var splitup = extractSplitup();
  var fromhr = nvl(window.localStorage.getItem("fromhr"), mConst().fromhrDef);
  var tohr = nvl(window.localStorage.getItem("tohr"), mConst().tohrDef);
  var frommin = nvl(window.localStorage.getItem("frommin"), mConst().fromminDef);
  var tomin = nvl(window.localStorage.getItem("tomin"), mConst().tominDef);
  var smartOn = nvl(window.localStorage.getItem("smart"), mConst().smartDef);

  var timePoint = new Date(base);
  var body = "";

  for (var i = 0; i < splitup.length; i++) {
    if (splitup[i] === "") {
      continue;
    }
    body = body + timePoint.format("hh:mm") + "," + splitup[i] + lb;
    timePoint = timePoint.addMinutes(mConst().sampleIntervalMins);
  }

  // Add smart alarm info into CSV data
  if (smartOn) {
    body = body + fromhr + ":" + frommin + ",START" + lb + tohr + ":" + tomin + ",END" + lb;
    if (goneoff != "N") {
      var goneoffstr = goneoff.substr(0, 2) + ":" + goneoff.substr(2, 2);
      body = body + goneoffstr + ",ALARM" + lb;
    }
  }
  return body;
}