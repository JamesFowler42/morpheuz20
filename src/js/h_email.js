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

/*global generateCopyLinkData, mLang, buildUrl, mConst, buildEmailJsonString, extractSplitup, sendMailViaServer, getWithDef, setNoDef, getNoDef */
/*exported automaticEmailExport */

/*
 * Call the automatic email export
 */
function automaticEmailExport() {

  try {

    // Find out config information
    var emailto = getWithDef("emailto", "");
    var doEmail = getWithDef("doemail", "N");

    // Escape if not configured
    if (emailto === "" || doEmail === "N") {
      console.log("automatic email deactivated");
      setNoDef("estat", mLang().disabled);
      return;
    }

    var base = parseInt(getNoDef("base"), 10);
    var splitup = extractSplitup();
    var smartOn = getWithDef("smart", mConst().smartDef);
    var fromhr = getWithDef("fromhr", mConst().fromhrDef);
    var frommin = getWithDef("frommin", mConst().fromminDef);
    var tohr = getWithDef("tohr", mConst().tohrDef);
    var tomin = getWithDef("tomin", mConst().tominDef);
    var goneoff = getWithDef("goneOff", "N");
    var snoozes = getWithDef("snoozes", "0");

    // Extract data
    var cpy = generateCopyLinkData(base, splitup, smartOn, fromhr, frommin, tohr, tomin, goneoff, snoozes);

    var url = buildUrl("Y");

    var email = buildEmailJsonString(emailto, base, url, cpy);

    // Send to server and await response
    setNoDef("estat", mLang().sending);
    sendMailViaServer(email, function(stat, resp) {
      if (stat === 1) {
        setNoDef("estat", mLang().ok);
      } else {
        setNoDef("estat", resp);
      }
    });

  } catch (err) {
    setNoDef("estat", err.message);
  }
}