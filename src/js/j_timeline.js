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

/*global calculateStats, window, mLang, makeGetAjaxCall, mConst, nvl, buildRecommendationPhrase, extractSplitup, mCommonConst, hrsmin */
/*exported addSmartAlarmPin, addBedTimePin, getQuoteOfTheDay, deleteUserPin, addSummaryPin */

/*
 * Ensure that we only insert one pin per day for each type and they move if we reset
 * using the base date as a unique identifier.
 */
function getPinId(base, type) {
  return "morpheuz-" + type + "-" + base.format("yyyyMMdd");
}

/*
 * Add a smart alarm pin when wakeup occurs (triggered when the gone off data is sent)
 */
function addSmartAlarmPin() {
  
  var baseStr = window.localStorage.getItem("base");
  var base = new Date(parseInt(baseStr,10));

  var stats = calculateStats(base, nvl(window.localStorage.getItem("goneOff"), "N"), extractSplitup());
  if (stats.tends === null) {
    console.log("addSmartAlarmPin: stats couldn't be calculated");
    return;
  }

  var alarmTime = stats.tends;
  var quote = window.localStorage.getItem("quote");
  var fromhr = window.localStorage.getItem("fromhr");
  var tohr = window.localStorage.getItem("tohr");
  var frommin = window.localStorage.getItem("frommin");
  var tomin = window.localStorage.getItem("tomin");

  var body = quote;
  
  var pin = {
    "id" : getPinId(base,"sa"),
    "time" : alarmTime.toISOString(),
    "layout" : {
      "type" : "genericPin",
      "title" : mLang().sa,
      "subtitle" : parseInt(fromhr,10) + ":" + frommin + " - " + parseInt(tohr,10) + ":" + tomin,
      "tinyIcon" : "system://images/ALARM_CLOCK",
      "backgroundColor" : "#00AAFF",
      "body" : body
    },
    "actions" : [ {
      "title" : mLang().startM,
      "type" : "openWatchApp",
      "launchCode" : 0
    } ]
  };

  console.log('Inserting pin: ' + JSON.stringify(pin));

  insertUserPin(pin, function(responseText) {
    console.log('Result: ' + responseText);
  });

}

/*
 * Add a bed time pin when we're due to go to sleep (triggered on reset and placed ahead of time)
 */
function addBedTimePin(base) {

  var auto = window.localStorage.getItem("autoReset");

  var baseDt = new Date(base);
  var bedTime = baseDt.addMinutes(24 * 60);
  var reminderTime = baseDt.addMinutes(23 * 60 + 30);

  var quote = window.localStorage.getItem("quote");
  
  var pin = null;
  
  if (auto === null || auto === "0") {
    // Suggested bed-time
    pin = {
      "id" : getPinId(baseDt,"bt"),
      "time" : bedTime.toISOString(),
      "layout" : {
        "type" : "genericPin",
        "title" : mLang().bedTime,
        "subtitle" : mLang().suggested,
        "tinyIcon" : "system://images/NOTIFICATION_REMINDER",
        "backgroundColor" : "#00AAFF",
        "body" : quote
      },
      "actions" : [ {
        "title" : mLang().bedNow,
        "type" : "openWatchApp",
        "launchCode" : 1
      } ]
    };
  } else {
    // Automatic bed time pin
    pin = {
      "id" : getPinId(baseDt,"bt"),
      "time" : bedTime.toISOString(),
      "layout" : {
        "type" : "genericPin",
        "title" : mLang().bedTime,
        "subtitle" : mLang().automatic,
        "tinyIcon" : "system://images/SCHEDULED_EVENT",
        "backgroundColor" : "#00AAFF",
        "body" : quote
      },
      "reminders": [
        {
          "time": reminderTime.toISOString(),
          "layout": {
            "type": "genericReminder",
            "tinyIcon": "system://images/SCHEDULED_EVENT",
            "title": mLang().bedtimeIn30Mins
          }
        } 
      ],
      "actions" : [ {
        "title" : mLang().bedNow,
        "type" : "openWatchApp",
        "launchCode" : 1
      }, {
        "title" : mLang().cancelBed,
        "type" : "openWatchApp",
        "launchCode" : 2
      } ]
    };
  }

  console.log('Inserting pin: ' + JSON.stringify(pin));

  insertUserPin(pin, function(responseText) {
    console.log('Result: ' + responseText);
  });

}

/*
 * Add the sleep summary pin (called at the same time as the smart alarm pin and during summary for when the smart alarm is not set)
 */
function addSummaryPin(atAlarmTime) {
  
  var goneOff = nvl(window.localStorage.getItem("goneOff"), "N");
  
  // At summary time and the alarm has gone off then this has already been done. Run away and don't tell anyone.
  if (!atAlarmTime && goneOff !== "N") {
    return;
  }
  
  var baseStr = window.localStorage.getItem("base");
  var base = new Date(parseInt(baseStr,10));
  
  var stats = calculateStats(base, goneOff, extractSplitup());
  if (stats.tends === null) {
    console.log("addSummaryPin: stats couldn't be calculated");
    return;
  }
  
  // We add one minute, just so as the smart alarm and summary pin occur in the same order always
  var summaryTime = stats.tends.addMinutes(1);

  var age = nvl(window.localStorage.getItem("age"), "");

  var body = buildRecommendationPhrase(age, stats);

  var pin = {
    "id" : getPinId(base,"su"),
    "time" : summaryTime.toISOString(),
    "layout" : {
      "type" : "genericPin",
      "title" : mLang().summary,
      "subtitle": hrsmin(stats.total),
      "tinyIcon" : "system://images/GLUCOSE_MONITOR",
      "backgroundColor" : "#00AAFF",
      "body" : body
    }
  };

  console.log('Inserting pin: ' + JSON.stringify(pin));

  insertUserPin(pin, function(responseText) {
    console.log('Result: ' + responseText);
  });

}

/*
 * Get some interesting body text
 */
function getQuoteOfTheDay() {

  makeGetAjaxCall(mConst().quotesUrl + "?v=" + new Date().getTime(), function(resp) {
    if (resp && resp.status === 1) {
      var obj = JSON.parse(resp.data);
      var ind = Math.round(Math.random() * obj.length);
      var quote = obj[ind];
      console.log('quote:' + quote);
      window.localStorage.setItem("quote", quote);
    }
  });
}

/*
 * Taken directly from Pebble's examples
 */
/** ***************************** timeline lib ******************************** */

// The timeline public URL root
var API_URL_ROOT = 'https://timeline-api.getpebble.com/';

/**
 * Send a request to the Pebble public web timeline API.
 * 
 * @param pin
 *          The JSON pin to insert. Must contain 'id' field.
 * @param type
 *          The type of request, either PUT or DELETE.
 * @param callback
 *          The callback to receive the responseText after the request has
 *          completed.
 */
function timelineRequest(pin, type, callback) {
  // User or shared?
  var url = API_URL_ROOT + 'v1/user/pins/' + pin.id;

  // Create XHR
  var xhr = new XMLHttpRequest();
  xhr.onload = function() {
    console.log('timeline: response received: ' + this.responseText);
    callback(this.responseText);
  };
  xhr.open(type, url);

  // Get token
  Pebble.getTimelineToken(function(token) {
    // Add headers
    xhr.setRequestHeader('Content-Type', 'application/json');
    xhr.setRequestHeader('X-User-Token', '' + token);
    console.log("X-User-Token:" + token);

    // Send
    xhr.send(JSON.stringify(pin));
    console.log('timeline: request sent.');
  }, function(error) {
    console.log('timeline: error getting timeline token: ' + error);
  });
}

/**
 * Insert a pin into the timeline for this user.
 * 
 * @param pin
 *          The JSON pin to insert.
 * @param callback
 *          The callback to receive the responseText after the request has
 *          completed.
 */
function insertUserPin(pin, callback) {
  timelineRequest(pin, 'PUT', callback);
}

/**
 * Delete a pin from the timeline for this user.
 * 
 * @param pin
 *          The JSON pin to delete.
 * @param callback
 *          The callback to receive the responseText after the request has
 *          completed.
 */
function deleteUserPin(pin, callback) {
  timelineRequest(pin, 'DELETE', callback);
}

/** *************************** end timeline lib ****************************** */


