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

/* global window */

(function() {
  'use strict';

  var MorpheuzUtil = require("./morpheuzUtil");
  var MorpheuzAjax = require("./morpheuzAjax");
  var MorpheuzConfig = require("./morpheuzConfig");
  var MorpheuzPushover = require("./morpheuzPushover");
  var MorpheuzUsage = require("./morpheuzUsage");
  var MorpheuzIFTTT = require("./morpheuzIFTTT");
  var MorpheuzHue = require("./morpheuzHue");
  var MorpheuzTimeline = require("./morpheuzTimeline");
  var MorpheuzEmail = require("./morpheuzEmail");
  var MorpheuzSWP = require("./morpheuzSWP");
  var MorpheuzCommon = require("./morpheuzCommon");

  /*
   * Reset log
   */
  function resetWithPreserve() {
    console.log("resetWithPreserve");

    // Define keep list
    var keep = [ {
      n : "version",
      d : MorpheuzConfig.mConst().versionDef
    }, {
      n : "smart",
      d : MorpheuzConfig.mConst().smartDef
    }, {
      n : "fromhr",
      d : MorpheuzConfig.mConst().fromhrDef
    }, {
      n : "frommin",
      d : MorpheuzConfig.mConst().fromminDef
    }, {
      n : "tohr",
      d : MorpheuzConfig.mConst().tohrDef
    }, {
      n : "tomin",
      d : MorpheuzConfig.mConst().tominDef
    }, {
      n : "emailto",
      d : ""
    }, {
      n : "pouser",
      d : ""
    }, {
      n : "postat",
      d : ""
    }, {
      n : "potoken",
      d : ""
    }, {
      n : "swpdo",
      d : ""
    }, {
      n : "swpstat",
      d : ""
    }, {
      n : "exptime",
      d : ""
    }, {
      n : "usage",
      d : "Y"
    }, {
      n : "lazarus",
      d : "Y"
    }, {
      n : "autoReset",
      d : "0"
    }, {
      n : "quote",
      d : ""
    }, {
      n : "lifx-token",
      d : ""
    }, {
      n : "lifx-time",
      d : MorpheuzConfig.mConst().lifxTimeDef
    }, {
      n : "hueip",
      d : ""
    }, {
      n : "hueusername",
      d : ""
    }, {
      n : "hueid",
      d : ""
    }, {
      n : "ifkey",
      d : ""
    }, {
      n : "ifserver",
      d : ""
    }, {
      n : "ifstat",
      d : ""
    }, {
      n : "age",
      d : ""
    }, {
      n : "doemail",
      d : ""
    }, {
      n : "estat",
      d : ""
    }, {
      n : "lat",
      d : ""
    }, {
      n : "long",
      d : ""
    } ];

    // Remember the keep list
    for (var i = 0; i < keep.length; i++) {
      var ki = keep[i];
      ki.v = MorpheuzUtil.getNoDef(ki.n);
    }

    // Clear memory
    window.localStorage.clear();
    clearPoints();

    // Restore from keep list
    for (var j = 0; j < keep.length; j++) {
      var kj = keep[j];
      MorpheuzUtil.setWithDef(kj.n, kj.v, kj.d);
    }
  }

  /*
   * There have been various reports of data not being cleared for the next day
   * This is about as explicit as can be.
   */
  function clearPoints() {
    for (var i = 0; i < MorpheuzConfig.mConst().limit; i++) {
      var entry = "P" + i;
      MorpheuzUtil.setNoDef(entry, "-1");
    }
  }

  /*
   * Store data returned from the watch
   */
  function storePointInfo(point, biggest) {
    var entry = "P" + point;
    if (biggest === 0) // Don't pass -1 across the link but 0 really means null
      biggest = -1; // Null
    else if (biggest === 5000)
      biggest = -2; // Ignored by user
    MorpheuzUtil.setNoDef(entry, biggest);
  }

  /*
   * Found location
   */
  function locationSuccess(pos) {
    var pLat = pos.coords.latitude.toFixed(1);
    var pLong = pos.coords.longitude.toFixed(1);
    console.log('lat=' + pLat + ', long=' + pLong);
    MorpheuzUtil.setNoDef("lat", pLat);
    MorpheuzUtil.setNoDef("long", pLong);
  }

  /*
   * No location, or forbidden
   */
  function locationError(err) {
    console.log('location error (' + err.code + '): ' + err.message);
    MorpheuzUtil.setNoDef("lat", "");
    MorpheuzUtil.setNoDef("long", "");
  }

  /*
   * Process ready from the watch
   */
  Pebble.addEventListener("ready", function(e) {
    console.log("ready");

    var smartStr = MorpheuzUtil.getNoDef("smart");
    if (smartStr === null || smartStr === "null") {
      resetWithPreserve();
    }
    MorpheuzTimeline.getQuoteOfTheDay();

    // Choose options about the data returned
    var options = {
      enableHighAccuracy : true,
      maximumAge : 10000,
      timeout : 10000
    };

    // Request current position
    navigator.geolocation.getCurrentPosition(locationSuccess, locationError, options);

  });

  /*
   * Return a control value back ACK
   */
  function callWatchApp(ctrlVal) {
    function decodeKeyCtrl(ctrlVal, keyVal, name) {
      return (ctrlVal & keyVal) ? name + " " : "";
    }
    console.log("ACK " + decodeKeyCtrl(ctrlVal, MorpheuzConfig.mConst().ctrlTransmitDone, "ctrlTransmitDone") + decodeKeyCtrl(ctrlVal, MorpheuzConfig.mConst().ctrlVersionDone, "ctrlVersionDone") + decodeKeyCtrl(ctrlVal, MorpheuzConfig.mConst().ctrlGoneOffDone, "ctrlGoneOffDone") + decodeKeyCtrl(ctrlVal, MorpheuzConfig.mConst().ctrlSnoozesDone, "ctrlSnoozesDone") + decodeKeyCtrl(ctrlVal, MorpheuzConfig.mConst().ctrlDoNext, "ctrlDoNext") + decodeKeyCtrl(ctrlVal, MorpheuzConfig.mConst().ctrlSetLastSent, "ctrlSetLastSent") + decodeKeyCtrl(ctrlVal, MorpheuzConfig.mConst().ctrlLazarus, "ctrlLazarus"));
    Pebble.sendAppMessage({
      "keyCtrl" : ctrlVal
    });
  }

  /*
   * Process sample from the watch
   */
  Pebble.addEventListener("appmessage", function(e) {

    // Build a response for the watchapp
    var ctrlVal = 0;

    // Incoming version number
    if (typeof e.payload.keyVersion !== "undefined") {
      var version = parseInt(e.payload.keyVersion, 10);
      console.log("MSG version=" + version);
      MorpheuzUtil.setNoDef("version", version);
      ctrlVal = ctrlVal | MorpheuzConfig.mConst().ctrlVersionDone;
      var lazarus = MorpheuzUtil.getNoDef("lazarus");
      if (lazarus !== "N") {
        ctrlVal = ctrlVal | MorpheuzConfig.mConst().ctrlLazarus;
      }
    }

    // Incoming origin timestamp - this is a reset
    if (typeof e.payload.keyBase !== "undefined") {
      var base = parseInt(e.payload.keyBase, 10);
      console.log("MSG base (watch)=" + base);
      // Watch delivers local time in seconds...
      base = base * 1000;
      resetWithPreserve();
      MorpheuzUtil.setNoDef("base", base);
      ctrlVal = ctrlVal | MorpheuzConfig.mConst().ctrlDoNext | MorpheuzConfig.mConst().ctrlSetLastSent;
      MorpheuzIFTTT.iftttMakerInterfaceBedtime();
    }

    // Incoming from value (first time for smart alarm)
    if (typeof e.payload.keyFrom !== "undefined") {
      var from = parseInt(e.payload.keyFrom, 10);
      var fromhr = MorpheuzConfig.mConst().fromhrDef;
      var frommin = MorpheuzConfig.mConst().fromminDef;
      var fsmart = MorpheuzConfig.mConst().smartDef;
      if (from !== -1) {
        var fhours = Math.floor(from / 60);
        var fminutes = from - fhours * 60;
        fromhr = MorpheuzCommon.fixLen(String(fhours));
        frommin = MorpheuzCommon.fixLen(String(fminutes));
        fsmart = "Y";
      }
      MorpheuzUtil.setNoDef("fromhr", fromhr);
      MorpheuzUtil.setNoDef("frommin", frommin);
      MorpheuzUtil.setNoDef("smart", fsmart);
      console.log("MSG from=" + from + ", smart=" + fsmart + ", fromhr=" + fromhr + ", frommin=" + frommin);
      ctrlVal = ctrlVal | MorpheuzConfig.mConst().ctrlDoNext | MorpheuzConfig.mConst().ctrlSetLastSent;
    }

    // Incoming to value (second time for smart alarm)
    if (typeof e.payload.keyTo !== "undefined") {
      var to = parseInt(e.payload.keyTo, 10);
      var tohr = MorpheuzConfig.mConst().tohrDef;
      var tomin = MorpheuzConfig.mConst().tominDef;
      var tsmart = MorpheuzConfig.mConst().smartDef;
      if (to !== -1) {
        var thours = Math.floor(to / 60);
        var tminutes = to - thours * 60;
        tohr = MorpheuzCommon.fixLen(String(thours));
        tomin = MorpheuzCommon.fixLen(String(tminutes));
        tsmart = "Y";
      }
      MorpheuzUtil.setNoDef("tohr", tohr);
      MorpheuzUtil.setNoDef("tomin", tomin);
      MorpheuzUtil.setNoDef("smart", tsmart);
      console.log("MSG to=" + to + ", smart=" + tsmart + ", tohr=" + tohr + ", tomin=" + tomin);
      ctrlVal = ctrlVal | MorpheuzConfig.mConst().ctrlDoNext | MorpheuzConfig.mConst().ctrlSetLastSent;
    }

    // Incoming gone off value
    if (typeof e.payload.keyGoneoff !== "undefined") {
      var goneoffNum = parseInt(e.payload.keyGoneoff, 10);
      var goneoff = "N";
      if (goneoffNum !== 0) {
        var hours = Math.floor(goneoffNum / 60);
        var minutes = goneoffNum - hours * 60;
        var hoursStr = MorpheuzCommon.fixLen(String(hours));
        var minutesStr = MorpheuzCommon.fixLen(String(minutes));
        goneoff = hoursStr + minutesStr;
      }
      console.log("MSG goneoff=" + goneoff);
      var previousGoneOff = MorpheuzUtil.getNoDef("goneOff");
      MorpheuzUtil.setNoDef("goneOff", goneoff);
      ctrlVal = ctrlVal | MorpheuzConfig.mConst().ctrlGoneOffDone | MorpheuzConfig.mConst().ctrlDoNext;
      MorpheuzTimeline.addSummaryPin(true);
      if (goneoff !== previousGoneOff) {
        MorpheuzTimeline.addSmartAlarmPin();
        MorpheuzAjax.turnLifxLightsOn();
        MorpheuzHue.turnHueLightsOn();
        MorpheuzIFTTT.iftttMakerInterfaceAlarm();
      } else {
        console.log("Only summary pin redone - gone off repeated");
      }
    }

    // Incoming data point
    if (typeof e.payload.keyPoint !== "undefined") {
      var point = parseInt(e.payload.keyPoint, 10);
      var top = point >> 16;
      var bottom = point & 0xFFFF;
      console.log("MSG point=" + top + ", biggest=" + bottom);
      storePointInfo(top, bottom);
      ctrlVal = ctrlVal | MorpheuzConfig.mConst().ctrlDoNext | MorpheuzConfig.mConst().ctrlSetLastSent;
    }

    // Store the snoozes
    if (typeof e.payload.keySnoozes !== "undefined") {
      var snoozes = parseInt(e.payload.keySnoozes, 10);
      console.log("MSG snoozes=" + snoozes);
      MorpheuzUtil.setNoDef("snoozes", snoozes);
      ctrlVal = ctrlVal | MorpheuzConfig.mConst().ctrlSnoozesDone | MorpheuzConfig.mConst().ctrlDoNext;
    }

    // Incoming transmit to automatics
    if (typeof e.payload.keyTransmit !== "undefined") {
      console.log("MSG transmit");
      transmitMethods();
      ctrlVal = ctrlVal | MorpheuzConfig.mConst().ctrlTransmitDone;
    }

    // What auto reset are we doing today?
    if (typeof e.payload.keyAutoReset !== "undefined") {
      var autoReset = parseInt(e.payload.keyAutoReset, 10);
      console.log("MSG keyAutoReset=" + autoReset);
      MorpheuzUtil.setNoDef("autoReset", autoReset);
      ctrlVal = ctrlVal | MorpheuzConfig.mConst().ctrlDoNext | MorpheuzConfig.mConst().ctrlSetLastSent;
    }

    // Respond back to watchapp here - we need assured positive delivery -
    // cannot
    // trust that it has reached the phone - must make
    // sure it has reached and been processed by the Pebble App and Javascript
    if (ctrlVal !== 0) {
      callWatchApp(ctrlVal);
    }

  });

  /*
   * Transmit method list
   */
  function transmitMethods() {
    // Protect against double send without resetting
    var transmitDone = MorpheuzUtil.getNoDef("transmitDone");
    if (transmitDone !== null) {
      console.log("transmit already done");
      return;
    }

    // Sends
    MorpheuzUsage.googleAnalytics();
    MorpheuzPushover.pushoverTransmit();
    MorpheuzSWP.smartwatchProTransmit();
    MorpheuzIFTTT.iftttMakerInterfaceData();
    MorpheuzTimeline.addSummaryPin(false);
    MorpheuzTimeline.addBedTimePin();
    MorpheuzEmail.automaticEmailExport();

    // Protect and report time
    MorpheuzUtil.setNoDef("transmitDone", "done");
    MorpheuzUtil.setNoDef("exptime", new Date().format(MorpheuzConfig.mConst().displayDateFmt));
  }

  /*
   * Monitor the closing of the config/display screen so as we can do a save if
   * needed
   */
  Pebble.addEventListener("webviewclosed", function(e) {
    console.log("webviewclosed " + e.response);

    // Nothing returned
    if (e.response === null || typeof e.response === 'undefined') {
      console.log("no config returned");
      return;
    }

    // Even then a lack of trust is reasonable
    var configData;
    try {
      configData = JSON.parse(decodeURIComponent(e.response));
    } catch (err) {
      console.log("no config returned");
      return;
    }

    // Process
    if (configData.action === "save") {
      MorpheuzUtil.setNoDef("emailto", configData.emailto);
      MorpheuzUtil.setNoDef("pouser", configData.pouser);
      MorpheuzUtil.setNoDef("potoken", configData.potoken);
      MorpheuzUtil.setNoDef("swpdo", configData.swpdo);
      MorpheuzUtil.setNoDef("usage", configData.usage);
      MorpheuzUtil.setNoDef("lazarus", configData.lazarus);
      MorpheuzUtil.setNoDef("lifx-token", configData.lifxtoken);
      MorpheuzUtil.setNoDef("lifx-time", configData.lifxtime);
      MorpheuzUtil.setNoDef("hueip", configData.hueip);
      MorpheuzUtil.setNoDef("hueusername", configData.hueuser);
      MorpheuzUtil.setNoDef("hueid", configData.hueid);
      MorpheuzUtil.setNoDef("ifkey", configData.ifkey);
      MorpheuzUtil.setNoDef("ifserver", configData.ifserver);
      MorpheuzUtil.setNoDef("age", configData.age);
      MorpheuzUtil.setNoDef("doemail", configData.doemail);

      // Test if requested
      if (configData.testsettings === "Y") {
        console.log("Test settings requested");
        MorpheuzPushover.pushoverTransmit();
        MorpheuzAjax.turnLifxLightsOn();
        MorpheuzHue.turnHueLightsOn();
        MorpheuzIFTTT.iftttMakerInterfaceAlarm();
        MorpheuzIFTTT.iftttMakerInterfaceData();
        MorpheuzIFTTT.iftttMakerInterfaceBedtime();
        MorpheuzEmail.automaticEmailExport();
      }
    }
  });

  /*
   * Show the config/display page - this will show a graph and allow a reset
   */
  Pebble.addEventListener("showConfiguration", function(e) {
    Pebble.openURL(MorpheuzUtil.buildUrl("N"));
  });

  /*
   * Unclear if this serves a purpose at all
   */
  Pebble.addEventListener("configurationClosed", function(e) {
    console.log("configurationClosed");
  });

}());
