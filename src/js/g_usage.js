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

/**
 * Send a usage data
 */
function sendAnonymousUsageData() {
  try {
    // The api key for Morpheuz is held in a separate file, this file is not checked into
    // github. If it isn't defined then we skip this functionality. No reason why someone shouldn't 
    // build their own Morpheuz, but don't want the usage stats contaminated
    if (typeof ai === "undefined") {
      console.log("usage api key not found");
      return;
    }
    // If the user has requested not to record stats then honour that request.
    var usage = nvl(window.localStorage.getItem("usage"), "Y");
    if (usage !== "Y") {
      console.log("usage opted out");
      return;
    }
    // Collect the information to be sent to strap
    // It collects some of it's own, but only stuff sent is anonymous
    var smart = nvl(window.localStorage.getItem("smart"), mConst().smartDef);
    var version = nvl(window.localStorage.getItem("version"), mConst().versionDef);
    var pushover = pushoverConfigured() ? "Y" : "N";
    var health = smartwatchProConfigured() ? "Y" : "N";
    var hostua = getUserAgent();
    var actionUrl = "/v" + version + "?sa=" + smart + "&po=" + pushover + "&hk=" + health + "&ua=" + hostua;

    // Send the usage stats
    console.log("sendAnonymousUsageData: event=" + actionUrl);
    var k = ai();
    var apiKey = k.j + k.k + k.f;
    strap_log_event(actionUrl, apiKey);

  } catch (err) {
    console.log("strap failed");
  }
}