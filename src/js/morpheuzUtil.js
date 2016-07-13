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

  var MorpheuzCommon = require("./morpheuzCommon");
  var MorpheuzConfig = require("./morpheuzConfig");

  var MorpheuzUtil = {};

  /*
   * Which watch platform
   */
  MorpheuzUtil.getPlatform = function() {
    try {
      var wi = Pebble.getActiveWatchInfo();
      if (wi && wi.platform) {
        return wi.platform;
      } else {
        return "unknown";
      }
    } catch (err) {
      return "unknown";
    }
  };

  /*
   * Convert to fixed length hex string
   */
  MorpheuzUtil.toHexStr = function(number, length, maxValueCap, minValueCap) {
    var num = parseInt(number, 10);
    if (num > maxValueCap) {
      num = maxValueCap;
    } else if (num < minValueCap) {
      num = minValueCap;
    }
    var str = '' + num.toString(16);
    while (str.length < length)
      str = '0' + str;
    return str;
  };

  /*
   * Often used pick a local stored value with a default
   */
  MorpheuzUtil.getWithDef = function(vName, defVal) {
    return MorpheuzCommon.nvl(MorpheuzUtil.getNoDef(vName), defVal);
  };

  /*
   * Get local storage without a default
   */
  MorpheuzUtil.getNoDef = function(vName) {
    return window.localStorage.getItem(vName);
  };

  /*
   * Set local storage without a default
   */
  MorpheuzUtil.setNoDef = function(vName, vValue) {
    window.localStorage.setItem(vName, vValue);
  };

  /*
   * Set local storage with a default
   */
  MorpheuzUtil.setWithDef = function(vName, vValue, defVal) {
    MorpheuzUtil.setNoDef(vName, MorpheuzCommon.nvl(vValue, defVal));
  };

  /*
   * Extract splitup array from local storage
   */
  MorpheuzUtil.extractSplitup = function() {
    var splitup = [];
    for (var j = 0; j < MorpheuzConfig.mConst().limit; j++) {
      var entry = "P" + j;
      var valueStr = MorpheuzUtil.getNoDef(entry);
      if (valueStr === null) {
        splitup[j] = "-1";
      } else {
        splitup[j] = valueStr;
      }
    }
    return splitup;
  };

  /*
   * Build the url for the config and report display @param noset
   */
  MorpheuzUtil.buildUrl = function(noset) {
    // If the version is set, keep it, if not the provide a not ready screen
    var version = MorpheuzUtil.getWithDef("version", MorpheuzConfig.mConst().versionDef);
    if (parseInt(version, 10) < MorpheuzConfig.mConst().lowestVersion) {
      return MorpheuzConfig.mConst().urlNotReady;
    }

    // Gather the chart together
    var base = MorpheuzUtil.getNoDef("base");

    // Graph data passed as hex as this is shorter in urls
    // Pushover has a limited url length
    var graphx = "";
    for (var i = 0; i < MorpheuzConfig.mConst().limit; i++) {
      var entry = "P" + i;
      var valueStr = MorpheuzUtil.getNoDef(entry);
      if (valueStr === null || valueStr === "-1") {
        graphx += "fff";
      } else if (valueStr === "-2") {
        graphx += "ffe";
      } else {
        graphx += MorpheuzUtil.toHexStr(valueStr, 3, 4093, 0);
      }
    }

    var fromhr = MorpheuzUtil.getWithDef("fromhr", MorpheuzConfig.mConst().fromhrDef);
    var tohr = MorpheuzUtil.getWithDef("tohr", MorpheuzConfig.mConst().tohrDef);
    var frommin = MorpheuzUtil.getWithDef("frommin", MorpheuzConfig.mConst().fromminDef);
    var tomin = MorpheuzUtil.getWithDef("tomin", MorpheuzConfig.mConst().tominDef);
    var smart = MorpheuzUtil.getWithDef("smart", MorpheuzConfig.mConst().smartDef);
    var goneOff = MorpheuzUtil.getWithDef("goneOff", "N");
    var emailto = MorpheuzUtil.getWithDef("emailto", "");
    var token = Pebble.getAccountToken();
    var age = MorpheuzUtil.getWithDef("age", "");
    var snoozes = MorpheuzUtil.getWithDef("snoozes", 0);
    var pLat = MorpheuzUtil.getWithDef("lat", "");
    var pLong = MorpheuzUtil.getWithDef("long", "");
    var fault = MorpheuzUtil.getWithDef("fault", 0);

    var extra = "";
    if (noset === "N") {
      var pouser = MorpheuzUtil.getWithDef("pouser", "");
      var postat = MorpheuzUtil.getWithDef("postat", "");
      var potoken = MorpheuzUtil.getWithDef("potoken", "");
      var swpdo = MorpheuzUtil.getWithDef("swpdo", "");
      var swpstat = MorpheuzUtil.getWithDef("swpstat", "");
      var exptime = MorpheuzUtil.getWithDef("exptime", "");
      var lifxToken = MorpheuzUtil.getWithDef("lifx-token", "");
      var lifxTime = MorpheuzUtil.getWithDef("lifx-time", MorpheuzConfig.mConst().lifxTimeDef);
      if (lifxTime === "") {
        lifxTime = MorpheuzConfig.mConst().lifxTimeDef;
      }
      var usage = MorpheuzUtil.getWithDef("usage", "Y");
      var lazarus = MorpheuzUtil.getWithDef("lazarus", "Y");
      var hueip = MorpheuzUtil.getWithDef("hueip", "");
      var hueusername = MorpheuzUtil.getWithDef("hueusername", "");
      var hueid = MorpheuzUtil.getWithDef("hueid", "");
      var ifkey = MorpheuzUtil.getWithDef("ifkey", "");
      var ifserver = MorpheuzUtil.getWithDef("ifserver", "");
      var ifstat = MorpheuzUtil.getWithDef("ifstat", "");
      var doEmail = MorpheuzUtil.getWithDef("doemail", "");
      var estat = MorpheuzUtil.getWithDef("estat", "");

      extra = "&pouser=" + encodeURIComponent(pouser) + "&postat=" + encodeURIComponent(postat) + "&potoken=" + encodeURIComponent(potoken) + "&swpdo=" + swpdo + "&swpstat=" + encodeURIComponent(swpstat) + "&exptime=" + encodeURIComponent(exptime) + "&usage=" + usage + "&lazarus=" + lazarus + "&lifxtoken=" + lifxToken + "&lifxtime=" + lifxTime + "&hueip=" + hueip + "&hueuser=" + encodeURIComponent(hueusername) + "&hueid=" + hueid + "&ifkey=" + ifkey + "&ifserver=" + encodeURIComponent(ifserver) + "&ifstat=" + encodeURIComponent(ifstat) + "&doemail=" + doEmail + "&estat=" + encodeURIComponent(estat);
    }

    var url = MorpheuzConfig.mConst().url + version + ".html" + "?base=" + base + "&graphx=" + graphx + "&fromhr=" + fromhr + "&tohr=" + tohr + "&frommin=" + frommin + "&tomin=" + tomin + "&smart=" + smart + "&vers=" + version + "&goneoff=" + goneOff + "&emailto=" + encodeURIComponent(emailto) + "&token=" + token + "&age=" + age + "&noset=" + noset + "&zz=" + snoozes + "&lat=" + pLat + "&long=" + pLong + "&fault=" + fault + extra;

    console.log("url=" + url + " (len=" + url.length + ")");
    return url;
  };

  module.exports = MorpheuzUtil;

}());