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

(function() {
  'use strict';

  var MorpheuzUtil = require("./morpheuzUtil");
  var MorpheuzAjax = require("./morpheuzAjax");
  var MorpheuzConfig = require("./morpheuzConfig");
  var MorpheuzCommon = require("./morpheuzCommon");

  var MorpheuzSWP = {};

  /*
   * Is Smartwatch Pro configured?
   */
  MorpheuzSWP.smartwatchProConfigured = function() {
    var doSwp = MorpheuzUtil.getWithDef("swpdo", "N");
    return (doSwp === "Y");
  };

  /*
   * Transmit to smartwatch pro
   */
  MorpheuzSWP.smartwatchProTransmit = function() {
    try {
      var doSwp = MorpheuzUtil.getWithDef("swpdo", "N");
      if (doSwp !== "Y") {
        MorpheuzUtil.setNoDef("swpstat", MorpheuzConfig.mLang().disabled);
        console.log("smartwatchProTransmit: swpdo not set");
        return;
      }
      MorpheuzUtil.setNoDef("swpstat", MorpheuzConfig.mLang().sending);
      var stats = MorpheuzCommon.calculateStats(parseInt(MorpheuzUtil.getNoDef("base"), 10), MorpheuzUtil.getWithDef("goneOff", "N"), MorpheuzUtil.extractSplitup());
      if (stats.tbegin === null || stats.tends === null) {
        MorpheuzUtil.setNoDef("swpstat", MorpheuzConfig.mLang().cnc);
        console.log("smartwatchProTransmit: stats couldn't be calculated");
        return;
      }
      var token = Pebble.getAccountToken();
      var swpUrl = MorpheuzConfig.mConst().smartwatchProAPI + stats.tbegin.format(MorpheuzConfig.mConst().swpUrlDate) + "&ends=" + stats.tends.format(MorpheuzConfig.mConst().swpUrlDate) + "&at=" + token;
      console.log("smartwatchProTransmit: url=" + swpUrl);
      MorpheuzAjax.makeGetAjaxCall(swpUrl, function(resp) {
        console.log("smartwatchProTransmit: " + JSON.stringify(resp));
        if (resp.status !== 1) {
          MorpheuzUtil.setNoDef("swpdo", "N"); // Turn off send on error
          MorpheuzUtil.setNoDef("swpstat", JSON.stringify(resp.errors));
        } else {
          MorpheuzUtil.setNoDef("swpstat", MorpheuzConfig.mLang().ok);
        }
      });
    } catch (err) {
      MorpheuzUtil.setNoDef("swpdo", "N"); // Turn off send on error
      MorpheuzUtil.setNoDef("swpstat", err.message);
    }
  };

  module.exports = MorpheuzSWP;

}());
