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

/*global mConst, fixLen, pushoverTransmit, smartwatchProTransmit, addBedTimePin, addSmartAlarmPin, addSummaryPin, getQuoteOfTheDay, turnLifxLightsOn, turnHueLightsOn, iftttMakerInterfaceAlarm, iftttMakerInterfaceData, iftttMakerInterfaceBedtime, automaticEmailExport, googleAnalytics, toHexStr, getWithDef, getNoDef, setNoDef, setWithDef */

/*
 * Reset log
 */
function resetWithPreserve() {
  console.log("resetWithPreserve");

  // Define keep list
  var keep = [
              { n: "version", d: mConst().versionDef },
              { n: "smart", d: mConst().smartDef },
              { n: "fromhr", d: mConst().fromhrDef },
              { n: "frommin", d: mConst().fromminDef },
              { n: "tohr", d: mConst().tohrDef },
              { n: "tomin", d: mConst().tominDef },
              { n: "emailto", d: "" },
              { n: "pouser", d: "" },
              { n: "postat", d: "" },
              { n: "potoken", d: "" },
              { n: "swpdo", d: "" },
              { n: "swpstat", d: "" },
              { n: "exptime", d: "" },
              { n: "usage", d: "Y" },
              { n: "lazarus", d: "Y" },
              { n: "autoReset", d: "0" },
              { n: "quote", d: "" },
              { n: "lifx-token", d: "" },
              { n: "lifx-time", d: mConst().lifxTimeDef },
              { n: "hueip", d: "" },
              { n: "hueusername", d: "" },
              { n: "hueid", d: "" },
              { n: "ifkey", d: "" },
              { n: "ifserver", d: "" },
              { n: "ifstat", d: "" },
              { n: "age", d: "" },
              { n: "doemail", d: "" },
              { n: "estat", d: "" },
              { n: "lat", d: "" },
              { n: "long", d: "" }
            ];

  // Remember the keep list
  for (var i = 0; i < keep.length; i++) {
    var ki = keep[i];
    ki.v = getNoDef(ki.n);
  }

  // Clear memory
  window.localStorage.clear();
  clearPoints();

  // Restore from keep list
  for (var j = 0; j < keep.length; j++) {
    var kj = keep[j];
    setWithDef(kj.n, kj.v, kj.d);
  }
}

/*
 * There have been various reports of data not being cleared for the next day
 * This is about as explicit as can be.
 */
function clearPoints() {
  for (var i = 0; i < mConst().limit; i++) {
    var entry = "P" + i;
    setNoDef(entry, "-1");
  }
}

/*
 * Which platforms require time zone correction
 */
function isTimeZoneCorrectionRequired() {
  return false;
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
  setNoDef(entry, biggest);
}

/*
 * Found location
 */
function locationSuccess(pos) {
  var pLat = pos.coords.latitude.toFixed(1);
  var pLong = pos.coords.longitude.toFixed(1);
  console.log('lat=' + pLat + ', long=' + pLong);
  setNoDef("lat", pLat);
  setNoDef("long", pLong);
}

/*
 * No location, or forbidden
 */
function locationError(err) {
  console.log('location error (' + err.code + '): ' + err.message);
  setNoDef("lat", "");
  setNoDef("long", "");
}

/*
 * Process ready from the watch
 */
Pebble.addEventListener("ready", function(e) {
  console.log("ready");

  var smartStr = getNoDef("smart");
  if (smartStr === null || smartStr === "null") {
    resetWithPreserve();
  }
  getQuoteOfTheDay();

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
  console.log("ACK " + decodeKeyCtrl(ctrlVal, mConst().ctrlTransmitDone, "ctrlTransmitDone") + decodeKeyCtrl(ctrlVal, mConst().ctrlVersionDone, "ctrlVersionDone") + decodeKeyCtrl(ctrlVal, mConst().ctrlGoneOffDone, "ctrlGoneOffDone") + decodeKeyCtrl(ctrlVal, mConst().ctrlSnoozesDone, "ctrlSnoozesDone") + decodeKeyCtrl(ctrlVal, mConst().ctrlDoNext, "ctrlDoNext") + decodeKeyCtrl(ctrlVal, mConst().ctrlSetLastSent, "ctrlSetLastSent") + decodeKeyCtrl(ctrlVal, mConst().ctrlLazarus, "ctrlLazarus"));
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
    setNoDef("version", version);
    ctrlVal = ctrlVal | mConst().ctrlVersionDone;
    var lazarus = getNoDef("lazarus");
    if (lazarus !== "N") {
      ctrlVal = ctrlVal | mConst().ctrlLazarus;
    }
  }

  // Incoming origin timestamp - this is a reset
  if (typeof e.payload.keyBase !== "undefined") {
    var base = parseInt(e.payload.keyBase, 10);
    console.log("MSG base (watch)=" + base);
    // Watch delivers local time in seconds...
    var offset = 0;
    if (isTimeZoneCorrectionRequired()) {
      offset = new Date().getTimezoneOffset() * 60;
    }
    console.log("offset = " + offset);
    base = (base + offset) * 1000;
    console.log("MSG base (js)=" + base);
    resetWithPreserve();
    setNoDef("base", base);
    ctrlVal = ctrlVal | mConst().ctrlDoNext | mConst().ctrlSetLastSent;
    iftttMakerInterfaceBedtime();
  }

  // Incoming from value (first time for smart alarm)
  if (typeof e.payload.keyFrom !== "undefined") {
    var from = parseInt(e.payload.keyFrom, 10);
    var fromhr = mConst().fromhrDef;
    var frommin = mConst().fromminDef;
    var fsmart = mConst().smartDef;
    if (from !== -1) {
      var fhours = Math.floor(from / 60);
      var fminutes = from - fhours * 60;
      fromhr = fixLen(String(fhours));
      frommin = fixLen(String(fminutes));
      fsmart = "Y";
    }
    setNoDef("fromhr", fromhr);
    setNoDef("frommin", frommin);
    setNoDef("smart", fsmart);
    console.log("MSG from=" + from + ", smart=" + fsmart + ", fromhr=" + fromhr + ", frommin=" + frommin);
    ctrlVal = ctrlVal | mConst().ctrlDoNext | mConst().ctrlSetLastSent;
  }

  // Incoming to value (second time for smart alarm)
  if (typeof e.payload.keyTo !== "undefined") {
    var to = parseInt(e.payload.keyTo, 10);
    var tohr = mConst().tohrDef;
    var tomin = mConst().tominDef;
    var tsmart = mConst().smartDef;
    if (to !== -1) {
      var thours = Math.floor(to / 60);
      var tminutes = to - thours * 60;
      tohr = fixLen(String(thours));
      tomin = fixLen(String(tminutes));
      tsmart = "Y";
    }
    setNoDef("tohr", tohr);
    setNoDef("tomin", tomin);
    setNoDef("smart", tsmart);
    console.log("MSG to=" + to + ", smart=" + tsmart + ", tohr=" + tohr + ", tomin=" + tomin);
    ctrlVal = ctrlVal | mConst().ctrlDoNext | mConst().ctrlSetLastSent;
  }

  // Incoming gone off value
  if (typeof e.payload.keyGoneoff !== "undefined") {
    var goneoffNum = parseInt(e.payload.keyGoneoff, 10);
    var goneoff = "N";
    if (goneoffNum !== 0) {
      var hours = Math.floor(goneoffNum / 60);
      var minutes = goneoffNum - hours * 60;
      var hoursStr = fixLen(String(hours));
      var minutesStr = fixLen(String(minutes));
      goneoff = hoursStr + minutesStr;
    }
    console.log("MSG goneoff=" + goneoff);
    var previousGoneOff = getNoDef("goneOff");
    setNoDef("goneOff", goneoff);
    ctrlVal = ctrlVal | mConst().ctrlGoneOffDone | mConst().ctrlDoNext;
    addSummaryPin(true);
    if (goneoff !== previousGoneOff) {
      addSmartAlarmPin();
      turnLifxLightsOn();
      turnHueLightsOn();
      iftttMakerInterfaceAlarm();
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
    ctrlVal = ctrlVal | mConst().ctrlDoNext | mConst().ctrlSetLastSent;
  }

  // Store the snoozes
  if (typeof e.payload.keySnoozes !== "undefined") {
    var snoozes = parseInt(e.payload.keySnoozes, 10);
    console.log("MSG snoozes=" + snoozes);
    setNoDef("snoozes", snoozes);
    ctrlVal = ctrlVal | mConst().ctrlSnoozesDone | mConst().ctrlDoNext;
  }

  // Incoming transmit to automatics
  if (typeof e.payload.keyTransmit !== "undefined") {
    console.log("MSG transmit");
    transmitMethods();
    ctrlVal = ctrlVal | mConst().ctrlTransmitDone;
  }

  // What auto reset are we doing today?
  if (typeof e.payload.keyAutoReset !== "undefined") {
    var autoReset = parseInt(e.payload.keyAutoReset, 10);
    console.log("MSG keyAutoReset=" + autoReset);
    setNoDef("autoReset", autoReset);
    ctrlVal = ctrlVal | mConst().ctrlDoNext | mConst().ctrlSetLastSent;
  }

  // Respond back to watchapp here - we need assured positive delivery - cannot
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
  var transmitDone = getNoDef("transmitDone");
  if (transmitDone !== null) {
    console.log("transmit already done");
    return;
  }

  // Sends
  googleAnalytics();
  pushoverTransmit();
  smartwatchProTransmit();
  iftttMakerInterfaceData();
  addSummaryPin(false);
  addBedTimePin();
  automaticEmailExport();

  // Protect and report time
  setNoDef("transmitDone", "done");
  setNoDef("exptime", new Date().format(mConst().displayDateFmt));
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
    setNoDef("emailto", configData.emailto);
    setNoDef("pouser", configData.pouser);
    setNoDef("potoken", configData.potoken);
    setNoDef("swpdo", configData.swpdo);
    setNoDef("usage", configData.usage);
    setNoDef("lazarus", configData.lazarus);
    setNoDef("lifx-token", configData.lifxtoken);
    setNoDef("lifx-time", configData.lifxtime);
    setNoDef("hueip", configData.hueip);
    setNoDef("hueusername", configData.hueuser);
    setNoDef("hueid", configData.hueid);
    setNoDef("ifkey", configData.ifkey);
    setNoDef("ifserver", configData.ifserver);
    setNoDef("age", configData.age);
    setNoDef("doemail", configData.doemail);

    // Test if requested
    if (configData.testsettings === "Y") {
      console.log("Test settings requested");
      pushoverTransmit();
      turnLifxLightsOn();
      turnHueLightsOn();
      iftttMakerInterfaceAlarm();
      iftttMakerInterfaceData();
      iftttMakerInterfaceBedtime();
      automaticEmailExport();
    }
  }
});

/*
 * Build the url for the config and report display @param noset
 */
function buildUrl(noset) {
  // If the version is set, keep it, if not the provide a not ready screen
  var version = getWithDef("version", mConst().versionDef);
  if (parseInt(version, 10) < mConst().lowestVersion) {
    return mConst().urlNotReady;
  }

  // Gather the chart together
  var base = getNoDef("base");

  // Graph data passed as hex as this is shorter in urls
  // Pushover has a limited url length
  var graphx = "";
  for (var i = 0; i < mConst().limit; i++) {
    var entry = "P" + i;
    var valueStr = getNoDef(entry);
    if (valueStr === null || valueStr === "-1") {
      graphx += "fff";
    } else if (valueStr === "-2") {
      graphx += "ffe";
    } else {
      graphx += toHexStr(valueStr, 3, 4093, 0);
    }
  }

  var fromhr = getWithDef("fromhr", mConst().fromhrDef);
  var tohr = getWithDef("tohr", mConst().tohrDef);
  var frommin = getWithDef("frommin", mConst().fromminDef);
  var tomin = getWithDef("tomin", mConst().tominDef);
  var smart = getWithDef("smart", mConst().smartDef);
  var goneOff = getWithDef("goneOff", "N");
  var emailto = getWithDef("emailto", "");
  var token = Pebble.getAccountToken();
  var age = getWithDef("age", "");
  var snoozes = getWithDef("snoozes", 0);
  var pLat = getWithDef("lat", "");
  var pLong = getWithDef("long", "");

  var extra = "";
  if (noset === "N") {
    var pouser = getWithDef("pouser", "");
    var postat = getWithDef("postat", "");
    var potoken = getWithDef("potoken", "");
    var swpdo = getWithDef("swpdo", "");
    var swpstat = getWithDef("swpstat", "");
    var exptime = getWithDef("exptime", "");
    var lifxToken = getWithDef("lifx-token", "");
    var lifxTime = getWithDef("lifx-time", mConst().lifxTimeDef);
    if (lifxTime === "") {
      lifxTime = mConst().lifxTimeDef;
    }
    var usage = getWithDef("usage", "Y");
    var lazarus = getWithDef("lazarus", "Y");
    var hueip = getWithDef("hueip", "");
    var hueusername = getWithDef("hueusername", "");
    var hueid = getWithDef("hueid", "");
    var ifkey = getWithDef("ifkey", "");
    var ifserver = getWithDef("ifserver", "");
    var ifstat = getWithDef("ifstat", "");
    var doEmail = getWithDef("doemail", "");
    var estat = getWithDef("estat", "");

    extra = "&pouser=" + encodeURIComponent(pouser) + "&postat=" + encodeURIComponent(postat) + "&potoken=" + encodeURIComponent(potoken) + "&swpdo=" + swpdo + "&swpstat=" + encodeURIComponent(swpstat) + "&exptime=" + encodeURIComponent(exptime) + "&usage=" + usage + "&lazarus=" + lazarus + "&lifxtoken=" + lifxToken + "&lifxtime=" + lifxTime + "&hueip=" + hueip + "&hueuser=" + encodeURIComponent(hueusername) + "&hueid=" + hueid + "&ifkey=" + ifkey + "&ifserver=" + encodeURIComponent(ifserver) + "&ifstat=" + encodeURIComponent(ifstat) + "&doemail=" + doEmail + "&estat=" + encodeURIComponent(estat);
  }

  var url = mConst().url + version + ".html" + "?base=" + base + "&graphx=" + graphx + "&fromhr=" + fromhr + "&tohr=" + tohr + "&frommin=" + frommin + "&tomin=" + tomin + "&smart=" + smart + "&vers=" + version + "&goneoff=" + goneOff + "&emailto=" + encodeURIComponent(emailto) + "&token=" + token + "&age=" + age + "&noset=" + noset + "&zz=" + snoozes + "&lat=" + pLat + "&long=" + pLong + extra;

  console.log("url=" + url + " (len=" + url.length + ")");
  return url;
}

/*
 * Show the config/display page - this will show a graph and allow a reset
 */
Pebble.addEventListener("showConfiguration", function(e) {
  Pebble.openURL(buildUrl("N"));
});

/*
 * Unclear if this serves a purpose at all
 */
Pebble.addEventListener("configurationClosed", function(e) {
  console.log("configurationClosed");
});
