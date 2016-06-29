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
  var MorpheuzConfig = require("./morpheuzConfig");
  var MorpheuzCommon = require("./morpheuzCommon");

  var MorpheuzEmail = {};

  /*
   * Call the automatic email export
   */
  MorpheuzEmail.automaticEmailExport = function() {

    try {

      // Find out config information
      var emailto = MorpheuzUtil.getWithDef("emailto", "");
      var doEmail = MorpheuzUtil.getWithDef("doemail", "N");

      // Escape if not configured
      if (emailto === "" || doEmail === "N") {
        console.log("automatic email deactivated");
        MorpheuzUtil.setNoDef("estat", MorpheuzConfig.mLang().disabled);
        return;
      }

      var base = parseInt(MorpheuzUtil.getNoDef("base"), 10);
      var splitup = MorpheuzUtil.extractSplitup();
      var smartOn = MorpheuzUtil.getWithDef("smart", MorpheuzConfig.mConst().smartDef);
      var fromhr = MorpheuzUtil.getWithDef("fromhr", MorpheuzConfig.mConst().fromhrDef);
      var frommin = MorpheuzUtil.getWithDef("frommin", MorpheuzConfig.mConst().fromminDef);
      var tohr = MorpheuzUtil.getWithDef("tohr", MorpheuzConfig.mConst().tohrDef);
      var tomin = MorpheuzUtil.getWithDef("tomin", MorpheuzConfig.mConst().tominDef);
      var goneoff = MorpheuzUtil.getWithDef("goneOff", "N");
      var snoozes = MorpheuzUtil.getWithDef("snoozes", "0");

      // Extract data
      var cpy = MorpheuzCommon.generateCopyLinkData(base, splitup, smartOn, fromhr, frommin, tohr, tomin, goneoff, snoozes);

      var url = MorpheuzUtil.buildUrl("Y");

      var email = MorpheuzCommon.buildEmailJsonString(emailto, base, url, cpy);

      // Send to server and await response
      MorpheuzUtil.setNoDef("estat", MorpheuzConfig.mLang().sending);
      MorpheuzCommon.sendMailViaServer(email, function(stat, resp) {
        if (stat === 1) {
          MorpheuzUtil.setNoDef("estat", MorpheuzConfig.mLang().ok);
        } else {
          MorpheuzUtil.setNoDef("estat", resp);
        }
      });

    } catch (err) {
      MorpheuzUtil.setNoDef("estat", err.message);
    }
  };

  module.exports = MorpheuzEmail;

}());
