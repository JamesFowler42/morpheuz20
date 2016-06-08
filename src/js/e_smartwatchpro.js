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

/*global mLang, mConst, makeGetAjaxCall, calculateStats, getWithDef, setNoDef, getNoDef */
/*exported smartwatchProConfigured, smartwatchProTransmit */

/*
 * Is Smartwatch Pro configured?
 */
function smartwatchProConfigured() {
  var doSwp = getWithDef("swpdo", "N");
  return (doSwp === "Y");
}

/*
 * Transmit to smartwatch pro
 */
function smartwatchProTransmit() {
  try {
    var doSwp = getWithDef("swpdo", "N");
    if (doSwp !== "Y") {
      setNoDef("swpstat", mLang().disabled);
      console.log("smartwatchProTransmit: swpdo not set");
      return;
    }
    setNoDef("swpstat", mLang().sending);
    var stats = calculateStats(parseInt(getNoDef("base"), 10), getWithDef("goneOff", "N"), extractSplitup());
    if (stats.tbegin === null || stats.tends === null) {
      setNoDef("swpstat", mLang().cnc);
      console.log("smartwatchProTransmit: stats couldn't be calculated");
      return;
    }
    var token = Pebble.getAccountToken();
    var swpUrl = mConst().smartwatchProAPI + stats.tbegin.format(mConst().swpUrlDate) + "&ends=" + stats.tends.format(mConst().swpUrlDate) + "&at=" + token;
    console.log("smartwatchProTransmit: url=" + swpUrl);
    makeGetAjaxCall(swpUrl, function(resp) {
      console.log("smartwatchProTransmit: " + JSON.stringify(resp));
      if (resp.status !== 1) {
        setNoDef("swpdo", "N"); // Turn off send on error
        setNoDef("swpstat", JSON.stringify(resp.errors));
      } else {
        setNoDef("swpstat", mLang().ok);
      }
    });
  } catch (err) {
    setNoDef("swpdo", "N"); // Turn off send on error
    setNoDef("swpstat", err.message);
  }
}

/*
 * Extract splitup array from local storage
 */
function extractSplitup() {
  var splitup = [];
  for (var j = 0; j < mConst().limit; j++) {
    var entry = "P" + j;
    var valueStr = getNoDef(entry);
    if (valueStr === null) {
      splitup[j] = "-1";
    } else {
      splitup[j] = valueStr;
    }
  }
  return splitup;
}
