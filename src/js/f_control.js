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

/*global window, nvl, mConst, fixLen, pushoverTransmit, smartwatchProTransmit, sendAnonymousUsageData, addBedTimePin, addSmartAlarmPin, getQuoteOfTheDay */

/*
 * Reset log
 */
function resetWithPreserve() {
  console.log("resetWithPreserve");
  var version = window.localStorage.getItem("version");
  var fromhr = window.localStorage.getItem("fromhr");
  var tohr = window.localStorage.getItem("tohr");
  var frommin = window.localStorage.getItem("frommin");
  var tomin = window.localStorage.getItem("tomin");
  var smart = window.localStorage.getItem("smart");
  var emailto = window.localStorage.getItem("emailto");
  var pouser = window.localStorage.getItem("pouser");
  var postat = window.localStorage.getItem("postat");
  var potoken = window.localStorage.getItem("potoken");
  var swpdo = window.localStorage.getItem("swpdo");
  var swpstat = window.localStorage.getItem("swpstat");
  var exptime = window.localStorage.getItem("exptime");
  var usage = window.localStorage.getItem("usage");
  var autoReset = window.localStorage.getItem("autoReset");
  var quote = window.localStorage.getItem("quote");
  window.localStorage.clear();
  window.localStorage.setItem("version", nvl(version, mConst().versionDef));
  window.localStorage.setItem("smart", nvl(smart, mConst().smartDef));
  window.localStorage.setItem("fromhr", nvl(fromhr, mConst().fromhrDef));
  window.localStorage.setItem("frommin", nvl(frommin, mConst().fromminDef));
  window.localStorage.setItem("tohr", nvl(tohr, mConst().tohrDef));
  window.localStorage.setItem("tomin", nvl(tomin, mConst().tominDef));
  window.localStorage.setItem("emailto", nvl(emailto, ""));
  window.localStorage.setItem("pouser", nvl(pouser, ""));
  window.localStorage.setItem("postat", nvl(postat, ""));
  window.localStorage.setItem("potoken", nvl(potoken, ""));
  window.localStorage.setItem("swpdo", nvl(swpdo, ""));
  window.localStorage.setItem("swpstat", nvl(swpstat, ""));
  window.localStorage.setItem("exptime", nvl(exptime, ""));
  window.localStorage.setItem("usage", nvl(usage, "Y"));
  window.localStorage.setItem("autoReset", nvl(autoReset, "0"));
  window.localStorage.setItem("quote", nvl(quote, ""));
}

/*
 * Which platform
 */
function isBasalt() {
  if (Pebble.getActiveWatchInfo) {
    var watchinfo = Pebble.getActiveWatchInfo();
    var platform = watchinfo.platform;
    return (platform === "basalt");
  } else {
    return false;
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
  window.localStorage.setItem(entry, biggest);
}

/*
 * Process ready from the watch
 */
Pebble.addEventListener("ready", function(e) {
  console.log("ready");
  var smartStr = window.localStorage.getItem("smart");
  if (smartStr === null || smartStr === "null") {
    resetWithPreserve();
    window.localStorage.setItem("smart", mConst().smartDef);
    window.localStorage.setItem("fromhr", mConst().fromhrDef);
    window.localStorage.setItem("frommin", mConst().fromminDef);
    window.localStorage.setItem("tohr", mConst().tohrDef);
    window.localStorage.setItem("tomin", mConst().tominDef);
  }
  getQuoteOfTheDay();
});

/*
 * Return a control value back ACK
 */
function callWatchApp(ctrlVal) {
  function decodeKeyCtrl(ctrlVal, keyVal, name) {
    return (ctrlVal & keyVal) ? name + " " : "";
  }
  console.log("ACK " + decodeKeyCtrl(ctrlVal, mConst().ctrlTransmitDone, "ctrlTransmitDone") + decodeKeyCtrl(ctrlVal, mConst().ctrlVersionDone, "ctrlVersionDone") + decodeKeyCtrl(ctrlVal, mConst().ctrlGoneOffDone, "ctrlGoneOffDone") + decodeKeyCtrl(ctrlVal, mConst().ctrlDoNext, "ctrlDoNext") + decodeKeyCtrl(ctrlVal, mConst().ctrlSetLastSent, "ctrlSetLastSent"));
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
    window.localStorage.setItem("version", version);
    ctrlVal = ctrlVal | mConst().ctrlVersionDone | mConst().ctrlDoNext;
  }

  // Incoming origin timestamp - this is a reset
  if (typeof e.payload.keyBase !== "undefined") {
    var base = parseInt(e.payload.keyBase, 10);
    console.log("MSG base (watch)=" + base);
    // Watch delivers local time in seconds...
    var offset = 0;
    if (!isBasalt()) {
      offset = new Date().getTimezoneOffset() * 60;
    }
    console.log("offset = " + offset);
    base = (base + offset) * 1000;
    console.log("MSG base (js)=" + base);
    resetWithPreserve();
    window.localStorage.setItem("base", base);
    ctrlVal = ctrlVal | mConst().ctrlDoNext | mConst().ctrlSetLastSent;
    addBedTimePin(base);
  }

  // Incoming from value (first time for smart alarm)
  if (typeof e.payload.keyFrom !== "undefined") {
    var from = parseInt(e.payload.keyFrom, 10);
    var fromhr = mConst().fromhrDef;
    var frommin = mConst().fromminDef;
    var smart = mConst().smartDef;
    if (from !== -1) {
      var hours = Math.floor(from / 60);
      var minutes = from - hours * 60;
      fromhr = fixLen(String(hours));
      frommin = fixLen(String(minutes));
      smart = "Y";
    }
    window.localStorage.setItem("fromhr", fromhr);
    window.localStorage.setItem("frommin", frommin);
    window.localStorage.setItem("smart", smart);
    console.log("MSG from=" + from + ", smart=" + smart + ", fromhr=" + fromhr + ", frommin=" + frommin);
    ctrlVal = ctrlVal | mConst().ctrlDoNext | mConst().ctrlSetLastSent;
  }

  // Incoming to value (second time for smart alarm)
  if (typeof e.payload.keyTo !== "undefined") {
    var to = parseInt(e.payload.keyTo, 10);
    var tohr = mConst().tohrDef;
    var tomin = mConst().tominDef;
    var smart = mConst().smartDef;
    if (to !== -1) {
      var hours = Math.floor(to / 60);
      var minutes = to - hours * 60;
      tohr = fixLen(String(hours));
      tomin = fixLen(String(minutes));
      smart = "Y";
    }
    window.localStorage.setItem("tohr", tohr);
    window.localStorage.setItem("tomin", tomin);
    window.localStorage.setItem("smart", smart);
    console.log("MSG to=" + to + ", smart=" + smart + ", tohr=" + tohr + ", tomin=" + tomin);
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
    turnLifxLightsOn();
    console.log("MSG goneoff=" + goneoff);
    window.localStorage.setItem("goneOff", goneoff);
    ctrlVal = ctrlVal | mConst().ctrlGoneOffDone | mConst().ctrlDoNext;
    addSmartAlarmPin();
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

  // Incoming transmit to automatics
  if (typeof e.payload.keyTransmit !== "undefined") {
    console.log("MSG transmit");
    transmitMethods();
    ctrlVal = ctrlVal | mConst().ctrlTransmitDone;
  }

  // Incoming transmit to automatics
  if (typeof e.payload.keyAutoReset !== "undefined") {
    var autoReset = parseInt(e.payload.keyAutoReset, 10);
    console.log("MSG keyAutoReset=" + autoReset);
    window.localStorage.setItem("autoReset", autoReset);
    ctrlVal = ctrlVal | mConst().ctrlDoNext | mConst().ctrlSetLastSent;
  }

  // Respond back to watchapp here - we need assured positive delivery - cannot trust that it has reached the phone - must make
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
  var transmitDone = window.localStorage.getItem("transmitDone");
  if (transmitDone !== null) {
    console.log("transmit already done");
    return;
  }

  // Sends
  pushoverTransmit();
  smartwatchProTransmit();
  sendAnonymousUsageData();

  // Protect and report time
  window.localStorage.setItem("transmitDone", "done");
  window.localStorage.setItem("exptime", new Date().format(mConst().displayDateFmt));
}

/*
 * Monitor the closing of the config/display screen so as we can do a reset if
 * needed
 */
Pebble.addEventListener("webviewclosed", function(e) {
  console.log("webviewclosed " + e.response);
  if (e.response === null)
    return;
  var dataElems = e.response.split("!");
  if (dataElems[0] === "reset") {
    window.localStorage.setItem("emailto", dataElems[7]);
    window.localStorage.setItem("pouser", dataElems[8]);
    window.localStorage.setItem("potoken", dataElems[10]);
    window.localStorage.setItem("swpdo", dataElems[12]);
    window.localStorage.setItem("usage", dataElems[13]);
    window.localStorage.setItem("lifx-token", dataElems[14]);
    window.localStorage.setItem("lifx-time", dataElems[15]);
  }
});

/*
 * Build the url for the config and report display @param noset
 */
function buildUrl(noset) {
  var base = window.localStorage.getItem("base");
  var graph = "";
  for (var i = 0; i < mConst().limit; i++) {
    var entry = "P" + i;
    var valueStr = window.localStorage.getItem(entry);
    if (valueStr === null) {
      graph = graph + "-1!";
    } else {
      graph = graph + valueStr + "!";
    }
  }
  var version = window.localStorage.getItem("version");
  if (!(parseInt(version, 10) >= 22))
    version = "22";

  var fromhr = nvl(window.localStorage.getItem("fromhr"), mConst().fromhrDef);
  var tohr = nvl(window.localStorage.getItem("tohr"), mConst().tohrDef);
  var frommin = nvl(window.localStorage.getItem("frommin"), mConst().fromminDef);
  var tomin = nvl(window.localStorage.getItem("tomin"), mConst().tominDef);
  var smart = nvl(window.localStorage.getItem("smart"), mConst().smartDef);
  var goneOff = nvl(window.localStorage.getItem("goneOff"), "N");
  var emailto = nvl(window.localStorage.getItem("emailto"), "");
  var pouser = "";
  var postat = "";
  var potoken = "";
  var token = "";
  var swpdo = "";
  var swpstat = "";
  var exptime = "";
  var usage = "";
  if (noset === "N") {
    pouser = nvl(window.localStorage.getItem("pouser"), "");
    postat = nvl(window.localStorage.getItem("postat"), "");
    potoken = nvl(window.localStorage.getItem("potoken"), "");
    token = Pebble.getAccountToken();
    swpdo = nvl(window.localStorage.getItem("swpdo"), "");
    swpstat = nvl(window.localStorage.getItem("swpstat"), "");
    exptime = nvl(window.localStorage.getItem("exptime"), "");
    lifxToken = nvl(window.localStorage.getItem("lifxtoken"), "");
    lifxTime = nvl(window.localStorage.getItem("lifxtime"), "");
    usage = nvl(window.localStorage.getItem("usage"), "Y");
  }

  var url = mConst().url + version + ".html?base=" + base + "&graph=" + graph + "&fromhr=" + fromhr + "&tohr=" + tohr + "&frommin=" + frommin + "&tomin=" + tomin + "&smart=" + smart + "&vers=" + version + "&goneoff=" + goneOff + "&emailto=" + encodeURIComponent(emailto) + "&pouser=" + encodeURIComponent(pouser) + "&postat=" + encodeURIComponent(postat) + "&potoken=" + encodeURIComponent(potoken) + "&noset=" + noset + "&token=" + token + "&swpdo=" + swpdo + "&swpstat=" + encodeURIComponent(swpstat) + "&exptime=" + encodeURIComponent(exptime) + "&usage=" + usage + "&lifxtoken=" + lifxToken + "&lifxtime=" + lifxTime;
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
