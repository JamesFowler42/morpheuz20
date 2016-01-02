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

/*global window, nvl, mConst, fixLen, pushoverTransmit, smartwatchProTransmit, addBedTimePin, addSmartAlarmPin, addSummaryPin, getQuoteOfTheDay, turnLifxLightsOn, turnHueLightsOn, iftttMakerInterfaceAlarm, iftttMakerInterfaceData, iftttMakerInterfaceBedtime */

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
  var lazarus = window.localStorage.getItem("lazarus");
  var autoReset = window.localStorage.getItem("autoReset");
  var quote = window.localStorage.getItem("quote");
  var lifxToken = window.localStorage.getItem("lifx-token");
  var lifxTime = window.localStorage.getItem("lifx-time");
  var hueip =  window.localStorage.getItem("hueip");
  var hueusername =  window.localStorage.getItem("hueusername");
  var hueid = window.localStorage.getItem("hueid");
  var ifkey = window.localStorage.getItem("ifkey");
  var ifserver = window.localStorage.getItem("ifserver");
  var ifstat = window.localStorage.getItem("ifstat");
  var age = window.localStorage.getItem("age");
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
  window.localStorage.setItem("lazarus", nvl(lazarus, "Y"));
  window.localStorage.setItem("autoReset", nvl(autoReset, "0"));
  window.localStorage.setItem("quote", nvl(quote, ""));
  window.localStorage.setItem("lifx-token", nvl(lifxToken, ""));
  window.localStorage.setItem("lifx-time", nvl(lifxTime, mConst().lifxTimeDef));
  window.localStorage.setItem("hueip", nvl(hueip, ""));
  window.localStorage.setItem("hueusername", nvl(hueusername, ""));
  window.localStorage.setItem("hueid", nvl(hueid, ""));
  window.localStorage.setItem("ifkey", nvl(ifkey, ""));
  window.localStorage.setItem("ifserver", nvl(ifserver, ""));
  window.localStorage.setItem("ifstat", nvl(ifstat, ""));
  window.localStorage.setItem("age", nvl(age, ""));
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
  console.log("ACK " + decodeKeyCtrl(ctrlVal, mConst().ctrlTransmitDone, "ctrlTransmitDone") + decodeKeyCtrl(ctrlVal, mConst().ctrlVersionDone, "ctrlVersionDone") + decodeKeyCtrl(ctrlVal, mConst().ctrlGoneOffDone, "ctrlGoneOffDone") + decodeKeyCtrl(ctrlVal, mConst().ctrlDoNext, "ctrlDoNext") + decodeKeyCtrl(ctrlVal, mConst().ctrlSetLastSent, "ctrlSetLastSent")+ decodeKeyCtrl(ctrlVal, mConst().ctrlLazarus, "ctrlLazarus"));
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
    ctrlVal = ctrlVal | mConst().ctrlVersionDone;
    var lazarus = window.localStorage.getItem("lazarus");
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
    window.localStorage.setItem("base", base);
    ctrlVal = ctrlVal | mConst().ctrlDoNext | mConst().ctrlSetLastSent;
    addBedTimePin(base);
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
    window.localStorage.setItem("fromhr", fromhr);
    window.localStorage.setItem("frommin", frommin);
    window.localStorage.setItem("smart", fsmart);
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
    window.localStorage.setItem("tohr", tohr);
    window.localStorage.setItem("tomin", tomin);
    window.localStorage.setItem("smart", tsmart);
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
    window.localStorage.setItem("goneOff", goneoff);
    ctrlVal = ctrlVal | mConst().ctrlGoneOffDone | mConst().ctrlDoNext;
    addSmartAlarmPin();
    turnLifxLightsOn();
    turnHueLightsOn();
    iftttMakerInterfaceAlarm();
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
  iftttMakerInterfaceData();
  addSummaryPin();

  // Protect and report time
  window.localStorage.setItem("transmitDone", "done");
  window.localStorage.setItem("exptime", new Date().format(mConst().displayDateFmt));
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
    window.localStorage.setItem("emailto", configData.emailto);
    window.localStorage.setItem("pouser", configData.pouser);
    window.localStorage.setItem("potoken", configData.potoken);
    window.localStorage.setItem("swpdo", configData.swpdo);
    window.localStorage.setItem("usage", configData.usage);
    window.localStorage.setItem("lazarus", configData.lazarus);
    window.localStorage.setItem("lifx-token", configData.lifxtoken);
    window.localStorage.setItem("lifx-time", configData.lifxtime);
    window.localStorage.setItem("hueip",  configData.hueip);
    window.localStorage.setItem("hueusername", configData.hueuser);
    window.localStorage.setItem("hueid", configData.hueid);
    window.localStorage.setItem("ifkey", configData.ifkey);
    window.localStorage.setItem("ifserver", configData.ifserver);
    window.localStorage.setItem("age", configData.age);

    // Test if requested
    if (configData.testsettings === "Y") {
      console.log("Test settings requested");
      pushoverTransmit(); 
      turnLifxLightsOn();
      turnHueLightsOn();
      iftttMakerInterfaceAlarm();
      iftttMakerInterfaceData();
      iftttMakerInterfaceBedtime();
    }
  }
});

/*
 * Build the url for the config and report display @param noset
 */
function buildUrl(noset) {
  // If the version is set, keep it, if not the provide a not ready screen
  var version = nvl(window.localStorage.getItem("version"), mConst().versionDef);
  if (parseInt(version, 10) < mConst().lowestVersion) {
    return mConst().urlNotReady;
  }
  
  // Gather the chart together
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

  var fromhr = nvl(window.localStorage.getItem("fromhr"), mConst().fromhrDef);
  var tohr = nvl(window.localStorage.getItem("tohr"), mConst().tohrDef);
  var frommin = nvl(window.localStorage.getItem("frommin"), mConst().fromminDef);
  var tomin = nvl(window.localStorage.getItem("tomin"), mConst().tominDef);
  var smart = nvl(window.localStorage.getItem("smart"), mConst().smartDef);
  var goneOff = nvl(window.localStorage.getItem("goneOff"), "N");
  var emailto = nvl(window.localStorage.getItem("emailto"), "");
  var token = Pebble.getAccountToken();
  var age = nvl(window.localStorage.getItem("age"), "");

  var extra = "";
  if (noset === "N") {
    var pouser = nvl(window.localStorage.getItem("pouser"), "");
    var postat = nvl(window.localStorage.getItem("postat"), "");
    var potoken = nvl(window.localStorage.getItem("potoken"), "");
    var swpdo = nvl(window.localStorage.getItem("swpdo"), "");
    var swpstat = nvl(window.localStorage.getItem("swpstat"), "");
    var exptime = nvl(window.localStorage.getItem("exptime"), "");
    var lifxToken = nvl(window.localStorage.getItem("lifx-token"), "");
    var lifxTime = nvl(window.localStorage.getItem("lifx-time"), mConst().lifxTimeDef);
    if (lifxTime === "") {
      lifxTime = mConst().lifxTimeDef;
    }
    var usage = nvl(window.localStorage.getItem("usage"), "Y");
    var lazarus = nvl(window.localStorage.getItem("lazarus"), "Y");
    var hueip =  nvl(window.localStorage.getItem("hueip"), "");
    var hueusername =  nvl(window.localStorage.getItem("hueusername"), "");
    var hueid = nvl(window.localStorage.getItem("hueid"), "");
    var ifkey = nvl(window.localStorage.getItem("ifkey"), "");
    var ifserver = nvl(window.localStorage.getItem("ifserver"), "");
    var ifstat = nvl(window.localStorage.getItem("ifstat"), "");
    extra = "&pouser=" + encodeURIComponent(pouser) + "&postat=" + encodeURIComponent(postat) + 
           "&potoken=" + encodeURIComponent(potoken) +  
           "&swpdo=" + swpdo + "&swpstat=" + encodeURIComponent(swpstat) + "&exptime=" + encodeURIComponent(exptime) + 
           "&usage=" + usage + "&lazarus=" + lazarus + "&lifxtoken=" + lifxToken + "&lifxtime=" + lifxTime +
           "&hueip=" + hueip + "&hueuser=" + encodeURIComponent(hueusername) + "&hueid=" + hueid +
           "&ifkey=" + ifkey + "&ifserver=" + encodeURIComponent(ifserver) + "&ifstat=" + encodeURIComponent(ifstat);
  }
  
  var url = mConst().url + version + ".html" + 
           "?base=" + base + "&graph=" + graph + "&fromhr=" + fromhr + "&tohr=" + tohr + "&frommin=" + frommin + "&tomin=" + tomin + 
           "&smart=" + smart + "&vers=" + version + "&goneoff=" + goneOff + "&emailto=" + encodeURIComponent(emailto) + "&token=" + token + "&age=" + age + 
           "&noset=" + noset + extra;
  
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
