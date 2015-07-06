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

/*global calculateStats, isBasalt, window, mLang, makeGetAjaxCall, mConst */
/*exported addSmartAlarmPin, addBedTimePin, getQuoteOfTheDay, deleteUserPin */

/*
 * Ensure that we only insert one pin per day for each type and they move if we reset
 * using the base date as a unique identifier.
 */
function getPinId(base, type) {
  return "morpheuz-" + type + "-" + base.format("yyyyMMdd");
}

/*
 * Add a smart alarm pin when wakeup occurs
 */
function addSmartAlarmPin() {
  if (!isBasalt()) {
    return;
  }

  var stats = calculateStats();
  if (stats.tends === null) {
    console.log("addSmartAlarmPin: stats couldn't be calculated");
    return;
  }

  var alarmTime = stats.tends;

  var quote = window.localStorage.getItem("quote");
  
  var baseStr = window.localStorage.getItem("base");
  var base = new Date(parseInt(baseStr,10));
  
  var fromhr = window.localStorage.getItem("fromhr");
  var tohr = window.localStorage.getItem("tohr");
  var frommin = window.localStorage.getItem("frommin");
  var tomin = window.localStorage.getItem("tomin");
  
  var body = mLang().earliest + fromhr + ":" + frommin + "\n" + 
             mLang().latest + tohr + ":" + tomin + "\n\n" + 
             quote;

  var pin = {
    "id" : getPinId(base,"sa"),
    "time" : alarmTime.toISOString(),
    "layout" : {
      "type" : "genericPin",
      "title" : mLang().sa,
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
 * Add a bed time pin when we're due to go to sleep
 */
function addBedTimePin(base) {
  if (!isBasalt()) {
    return;
  }

  var auto = window.localStorage.getItem("autoReset");
  if (auto === null || auto === "0") {
    return;
  }

  var baseDt = new Date(base);
  var bedTime = baseDt.addMinutes(24 * 60);

  var quote = window.localStorage.getItem("quote");

  var pin = {
    "id" : getPinId(baseDt,"bt"),
    "time" : bedTime.toISOString(),
    "layout" : {
      "type" : "genericPin",
      "title" : mLang().bedTime,
      "tinyIcon" : "system://images/SCHEDULED_EVENT",
      "backgroundColor" : "#00AAFF",
      "body" : quote
    },
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

  console.log('Inserting pin: ' + JSON.stringify(pin));

  insertUserPin(pin, function(responseText) {
    console.log('Result: ' + responseText);
  });

}

/*
 * Get some interesting body text
 */
function getQuoteOfTheDay() {
  if (!isBasalt()) {
    return;
  }
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
