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

(function() {
  'use strict';

  var MorpheuzUtil = require("./morpheuzUtil");
  var MorpheuzAjax = require("./morpheuzAjax");
  var MorpheuzConfig = require("./morpheuzConfig");
  var MorpheuzCommon = require("./morpheuzCommon");

  var MorpheuzTimeline = {};

  /*
   * Ensure that we only insert one pin per day for each type and they move if
   * we reset using the base date as a unique identifier.
   */
  function getPinId(base, type) {
    return "morpheuz-" + type + "-" + base.format("yyyyMMdd");
  }

  /*
   * Add a smart alarm pin when wakeup occurs (triggered when the gone off data
   * is sent)
   */
  MorpheuzTimeline.addSmartAlarmPin = function() {

    var baseStr = MorpheuzUtil.getNoDef("base");
    var base = new Date(parseInt(baseStr, 10));

    var stats = MorpheuzCommon.calculateStats(base, MorpheuzUtil.getWithDef("goneOff", "N"), MorpheuzUtil.extractSplitup());
    if (stats.tends === null) {
      console.log("addSmartAlarmPin: stats couldn't be calculated");
      return;
    }

    var alarmTime = stats.tends;
    var quote = MorpheuzUtil.getNoDef("quote");
    var fromhr = MorpheuzUtil.getNoDef("fromhr");
    var tohr = MorpheuzUtil.getNoDef("tohr");
    var frommin = MorpheuzUtil.getNoDef("frommin");
    var tomin = MorpheuzUtil.getNoDef("tomin");

    var body = quote;

    var pin = {
      "id" : getPinId(base, "sa"),
      "time" : alarmTime.toISOString(),
      "layout" : {
        "type" : "genericPin",
        "title" : MorpheuzConfig.mLang().sa,
        "subtitle" : parseInt(fromhr, 10) + ":" + frommin + " - " + parseInt(tohr, 10) + ":" + tomin,
        "tinyIcon" : "system://images/ALARM_CLOCK",
        "backgroundColor" : "#00AAFF",
        "body" : body
      },
      "actions" : [ {
        "title" : MorpheuzConfig.mLang().startM,
        "type" : "openWatchApp",
        "launchCode" : 0
      } ]
    };

    console.log('Inserting pin: ' + JSON.stringify(pin));

    insertUserPin(pin, function(responseText) {
      console.log('Result: ' + responseText);
    });

  };

  /*
   * Add a bed time pin when we're due to go to sleep (triggered on export
   * functions)
   */
  MorpheuzTimeline.addBedTimePin = function() {

    var baseStr = MorpheuzUtil.getNoDef("base");
    var base = new Date(parseInt(baseStr, 10));

    var auto = MorpheuzUtil.getNoDef("autoReset");

    var baseDt = new Date(base);
    var bedTime = baseDt.addMinutes(24 * 60);
    var reminderTime = baseDt.addMinutes(23 * 60 + 30);

    var quote = MorpheuzUtil.getNoDef("quote");

    var pin = null;

    if (auto === null || auto === "0") {
      // Suggested bed-time
      pin = {
        "id" : getPinId(baseDt, "bt"),
        "time" : bedTime.toISOString(),
        "layout" : {
          "type" : "genericPin",
          "title" : MorpheuzConfig.mLang().bedTime,
          "subtitle" : MorpheuzConfig.mLang().suggested,
          "tinyIcon" : "system://images/NOTIFICATION_REMINDER",
          "backgroundColor" : "#00AAFF",
          "body" : quote
        },
        "actions" : [ {
          "title" : MorpheuzConfig.mLang().bedNow,
          "type" : "openWatchApp",
          "launchCode" : 1
        } ]
      };
    } else {
      // Automatic bed time pin
      pin = {
        "id" : getPinId(baseDt, "bt"),
        "time" : bedTime.toISOString(),
        "layout" : {
          "type" : "genericPin",
          "title" : MorpheuzConfig.mLang().bedTime,
          "subtitle" : MorpheuzConfig.mLang().automatic,
          "tinyIcon" : "system://images/SCHEDULED_EVENT",
          "backgroundColor" : "#00AAFF",
          "body" : quote
        },
        "reminders" : [ {
          "time" : reminderTime.toISOString(),
          "layout" : {
            "type" : "genericReminder",
            "tinyIcon" : "system://images/SCHEDULED_EVENT",
            "title" : MorpheuzConfig.mLang().bedtimeIn30Mins
          }
        } ],
        "actions" : [ {
          "title" : MorpheuzConfig.mLang().bedNow,
          "type" : "openWatchApp",
          "launchCode" : 1
        }, {
          "title" : MorpheuzConfig.mLang().cancelBed,
          "type" : "openWatchApp",
          "launchCode" : 2
        } ]
      };
    }

    console.log('Inserting pin: ' + JSON.stringify(pin));

    insertUserPin(pin, function(responseText) {
      console.log('Result: ' + responseText);
    });

  };

  /*
   * Add the sleep summary pin (called at the same time as the smart alarm pin
   * and during summary for when the smart alarm is not set)
   */
  MorpheuzTimeline.addSummaryPin = function(atAlarmTime) {

    var goneOff = MorpheuzUtil.getWithDef("goneOff", "N");

    // At summary time and the alarm has gone off then this has already been
    // done. Run away and don't tell anyone.
    if (!atAlarmTime && goneOff !== "N") {
      return;
    }

    var baseStr = MorpheuzUtil.getNoDef("base");
    var base = new Date(parseInt(baseStr, 10));

    var stats = MorpheuzCommon.calculateStats(base, goneOff, MorpheuzUtil.extractSplitup());
    if (stats.tends === null) {
      console.log("addSummaryPin: stats couldn't be calculated");
      return;
    }

    // We add one minute, just so as the smart alarm and summary pin occur in
    // the same order always
    var summaryTime = stats.tends.addMinutes(1);

    var age = MorpheuzUtil.getWithDef("age", "");
    var snoozes = parseInt(MorpheuzUtil.getWithDef("snoozes", "0"), 10);
    if (isNaN(snoozes)) {
      snoozes = 0;
    }

    var rec = MorpheuzCommon.buildRecommendationPhrase(age, stats, snoozes);

    var actions = [];

    if (MorpheuzUtil.getPlatform() !== "aplite") {
      actions.push({
        "title" : MorpheuzConfig.mLang().showChart,
        "type" : "openWatchApp",
        "launchCode" : 3
      });
    }

    var pin = {
      "id" : getPinId(base, "su"),
      "time" : summaryTime.toISOString(),
      "layout" : {
        "type" : "genericPin",
        "title" : MorpheuzConfig.mLang().summary,
        "subtitle" : rec.total,
        "tinyIcon" : "system://images/GLUCOSE_MONITOR",
        "backgroundColor" : "#00AAFF",
        "body" : rec.summary
      },
      "actions" : actions
    };

    console.log('Inserting pin: ' + JSON.stringify(pin));

    insertUserPin(pin, function(responseText) {
      console.log('Result: ' + responseText);
    });

  };

  /*
   * Get some interesting body text
   */
  MorpheuzTimeline.getQuoteOfTheDay = function() {

    MorpheuzAjax.makeGetAjaxCall(MorpheuzConfig.mConst().quotesUrl + "?v=" + new Date().getTime(), function(resp) {
      if (resp && resp.status === 1) {
        var obj = JSON.parse(resp.data);
        var ind = Math.floor(Math.random() * obj.length);
        var quote = obj[ind];
        console.log('quote:' + quote);
        MorpheuzUtil.setNoDef("quote", quote);
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

  module.exports = MorpheuzTimeline;

}());
