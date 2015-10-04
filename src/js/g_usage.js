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

/*global nvl, window, mConst, getUserAgent, makeGetAjaxCall, getPlatform */
/*exported sendAnonymousUsageData */

/**
 * Send a usage data
 */
function sendAnonymousUsageData() {
  try {
    // If the user has requested not to record stats then honour that request.
    var usage = nvl(window.localStorage.getItem("usage"), "Y");
    if (usage !== "Y") {
      console.log("usage opted out");
      return;
    }
    var version = nvl(window.localStorage.getItem("version"), mConst().versionDef);
    var platform = getPlatform();
    var hostua = getUserAgent();
    var basicUrl = "v" + version + "-" + hostua + "-" + platform + mConst().usageSx;
    var actionUrl = mConst().usageUrl + basicUrl.toLowerCase() + "?v=" + new Date().getTime();

    // Send the usage stats
    console.log("sendAnonymousUsageData: event=" + actionUrl);
    makeGetAjaxCall(actionUrl, function(resp) {
      console.log("sendAnonymousUsageData resp= " + JSON.stringify(resp));
    });

  } catch (err) {
    console.log("usage unable to be stored");
  }
}