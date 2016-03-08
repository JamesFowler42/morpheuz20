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

/*global window, nvl, mConst */
/*exported googleAnalytics */

/*
 * Call googleAnalytics gathering which features of Morpheuz are being used
 * No personal data is collected, but the usage informs further development directions
 */
function googleAnalytics() {
 
  // morpheuz_smart_alarm
  var smartAlarmOn = nvl(window.localStorage.getItem("smart"), mConst().smartDef) !== mConst().smartDef;
  // morpheuz_ifttt
  var iftttOn = nvl(window.localStorage.getItem("ifkey"), "") !== "";
  // morpheuz_lifx
  var lifxOn = nvl(window.localStorage.getItem("lifx-token"), "") !== "";
  // morpheuz_hue
  var hueOn =  nvl(window.localStorage.getItem("hueip"), "") !== "";
  // morpheuz_email_address
  var emailOn = nvl(window.localStorage.getItem("emailto"), "") !== "";
  // morpheuz_pushover
  var pushoverOn = nvl(window.localStorage.getItem("potoken"), "") !== "";
  // morpheuz_auto_email
  var autoEmailOn = nvl(window.localStorage.getItem("doemail"), "") === "Y";
  // morpheuz_auto_healthkit
  var healthKitAutoExportOn = nvl(window.localStorage.getItem("swpdo"), "") === "Y";
  // morpheuz_age_entered
  var ageSupplied = nvl(window.localStorage.getItem("age"), "") !== "";
  // morpheuz_lazarus
  var lazarusActive = nvl(window.localStorage.getItem("lazarus"), "Y") === "Y";
  
  // Pass this as a custom dimension array
  var customDimension = [smartAlarmOn,iftttOn,lifxOn,hueOn,emailOn,pushoverOn,autoEmailOn,healthKitAutoExportOn,ageSupplied,lazarusActive];
  
  // Call analytics - this has been done like this so the analytics function can be spun off as a separate library later
  googleAnalyticsCall(customDimension);
}

/*
 * Store usage information using google analytics (only if the user permits)
 * customDimension is an array that is passed as cd1=xxx&cd2=yyy...
 */
function googleAnalyticsCall(customDimension) {
  
  try {
    
    // If the user doesn't want to do this then don't do it.
    var usage = nvl(window.localStorage.getItem("usage"), "Y");
    if (usage !== "Y") {
      console.log("googleAnalytics: disabled by user request");
      return;
    }
    
    // Pick up the version
    var version = nvl(window.localStorage.getItem("version"), "unknown");
    
    // Use the custom dimensions supplied to provide extra information
    var cd = "";
    try {
    if (customDimension && customDimension.constructor == Array) {
      for (var i = 0; i < customDimension.length; i++) {
        var dimensionIndex = i + 1;
        cd += "&cd" + dimensionIndex + "=" + customDimension[i];   
      }
    }
    } catch (e) {
      console.log("googleAnalytics: failed to get custom dimensions");
    }
    
    // Uniquely identify by account token
    var accountToken = "unknown";
    try {
      accountToken = Pebble.getAccountToken();
    } catch (e) {
      console.log("googleAnalytics: unable to get accountToken");
    }
    
    // Pick up as much platform information as possible
    // This will enable decisions on what features to add and what should take priority
    var platform = "unknown";
    var model = "unknown";
    var language = "unknown";
    var depth = "unknown";
    var screenres = "unknown";
    var firmware = "unknown";
    try {
      var wi = Pebble.getActiveWatchInfo();
      if (wi && wi.platform) {
        platform = wi.platform;
        if (platform === "aplite") {
          depth = "bw";
        } else {
          depth = "colour";
        }
        if (platform === "chalk") {
          screenres = "180x180";
        } else {
          screenres = "144x168";
        }
      }
      if (wi && wi.model) {
        model = wi.model;
      }
      if (wi && wi.language) {
        language = wi.language;
      }
      if (wi && wi.firmware && wi.firmware.major && wi.firmware.minor) {
        firmware = "v" + wi.firmware.major + "." + wi.firmware.minor;
        if (wi.firmware.patch) {
          firmware += "." + firmware.patch;
          if (wi.firmware.suffix) {
            firmware += "." + firmware.suffix;
          }
        }
      }
    } catch (e) {
      console.log("googleAnalytics: unable to get active watch info");
    }
   
    // User agent
    var userAgent = getUserAgent(model);
    
    // Build the google analytics api
    var msg = "v=1" +
              "&tid=UA-72769045-3" + 
              "&ds=app" +
              "&cid=" + accountToken + 
              "&t=event" +
              "&cd=" + platform +
              "&an=Morpheuz" +
              "&ec=" + userAgent +
              "&ea= " + platform +
              "&el=" + model + 
              "&ul=" + language + 
              "&av=" + version + 
              "&sd=" + depth +
              "&sr=" + screenres +
              "&dh=" + firmware +
              cd;
    
    console.log("googleAnalytics: " + msg);

    // Send this across to google
    // Do not wait for a response, but report one if it turns up
    var req = new XMLHttpRequest();
    req.open("POST", "https://ssl.google-analytics.com/collect", true);
    req.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
    req.onload = function() {
      if (req.readyState === 4 && req.status === 200) {
        console.log("googleAnalytics: OK");
      } else if (req.readyState === 4 && (req.status >= 300 && req.status <= 599)) {
        console.log("googleAnalytics: Failed");
      }
    };
    req.send(msg);
    
  } catch (err) {
    console.log("googleAnalytics: Failed to call google Analytics with " + err);
  }
  
}

/*
 * Work out user agent - if the model says qemu then it is emulator. Otherwise check the userAgent. If we cannot then it is probably Android.
 * iOS seems to always return a reasonable value.
 */
function getUserAgent(model) {
  if (model.match(/qemu/i)) {
    return "Emulator";
  }
  try {
    if (navigator && navigator.userAgent) {
      console.log("userAgent=" + navigator.userAgent);
      return (navigator.userAgent.match(/iPhone/i) || navigator.userAgent.match(/iPad/i) || navigator.userAgent.match(/iPod/i)) ? "iOS" : "Android";
    } else {
      return "Android";
    }
  } catch (err) {
    return "Android";
  }
}