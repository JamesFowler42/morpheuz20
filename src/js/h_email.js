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

/*global generateCopyLinkData, window, mLang, buildUrl, mConst, nvl, buildEmailJsonString, extractSplitup, sendMailViaServer */
/*exported automaticEmailExport */

/*
 * Call the automatic email export
 */
function automaticEmailExport() {

  try {
  
    // Find out config information
    var emailto = nvl(window.localStorage.getItem("emailto"), "");
    var doEmail = nvl(window.localStorage.getItem("doemail"), "N");
 
    // Escape if not configured
    if (emailto === "" || doEmail === "N") {
      console.log("automatic email deactivated");
      window.localStorage.setItem("estat", mLang().disabled);
      return;
    }
   
    var base = parseInt(window.localStorage.getItem("base"), 10);
    var splitup = extractSplitup();
    var smartOn = nvl(window.localStorage.getItem("smart"), mConst().smartDef);
    var fromhr = nvl(window.localStorage.getItem("fromhr"), mConst().fromhrDef);
    var frommin = nvl(window.localStorage.getItem("frommin"), mConst().fromminDef);
    var tohr = nvl(window.localStorage.getItem("tohr"), mConst().tohrDef);
    var tomin = nvl(window.localStorage.getItem("tomin"), mConst().tominDef);
    var goneoff = nvl(window.localStorage.getItem("goneOff"), "N");
    var snoozes = nvl(window.localStorage.getItem("snoozes"), "0");
    
    // Extract data
    var cpy = generateCopyLinkData(base, splitup, smartOn, fromhr, frommin, tohr, tomin, goneoff, snoozes);

    var url = buildUrl("Y");

    var email = buildEmailJsonString(emailto, base, url, cpy);

    // Send to server and await response
    window.localStorage.setItem("estat", mLang().sending);
    sendMailViaServer(email, function(stat, resp) {
      if (stat === 1) {
        window.localStorage.setItem("estat", mLang().ok);
      } else {
        window.localStorage.setItem("estat", resp);
      }
    });
    
  } catch (err) {
    window.localStorage.setItem("estat", err.message);
  }
}