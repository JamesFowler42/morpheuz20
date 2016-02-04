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

/*global window, nvl,  */
/*exported googleAnalytics */

/*
 * Store usage information using google analytics
 */
function googleAnalytics() {
  
  try {
    
    // If the user doesn't want to do this then don't do it.
    var usage = nvl(window.localStorage.getItem("usage"), "Y");
    if (usage !== "Y") {
      console.log("googleAnalytics: disabled by user request");
      return;
    }
    
    // Pick up the version
    var version = nvl(window.localStorage.getItem("version"), "unknown");
    
    // Uniquely identify by account token
    var accountToken = "unknown";
    try {
      accountToken = Pebble.getAccountToken();
    } catch (e) {
      console.log("googleAnalytics: unable to get accountToken");
    }
    
    // Pick up as much platform information as possible
    var platform = "unknown";
    var model = "unknown";
    var language = "unknown";
    var depth = "unknown";
    var screenres = "unknown";
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
              "&sr=" + screenres;
    
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