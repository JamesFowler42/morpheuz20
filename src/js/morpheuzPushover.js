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

  var MorpheuzPushover = {};

  /*
   * Is Pushover configured?
   */
  MorpheuzPushover.pushoverConfigured = function() {
    var pouser = MorpheuzUtil.getWithDef("pouser", "");
    var potoken = MorpheuzUtil.getWithDef("potoken", "");
    return (pouser !== "" && potoken !== "");
  };

  /**
   * Send a pushover message
   */
  MorpheuzPushover.pushoverTransmit = function() {
    try {
      var pouser = MorpheuzUtil.getWithDef("pouser", "");
      var potoken = MorpheuzUtil.getWithDef("potoken", "");
      if (pouser === "" || potoken === "") {
        MorpheuzUtil.setNoDef("postat", MorpheuzConfig.mLang().disabled);
        console.log("pushoverTransmit: potoken and/or pouser not set");
        return;
      }
      MorpheuzUtil.setNoDef("postat", MorpheuzConfig.mLang().sending);
      var base = MorpheuzUtil.getNoDef("base");
      var resetDate = new Date(parseInt(base, 10)).format(MorpheuzConfig.mConst().displayDateFmt);
      var urlToAttach = MorpheuzUtil.buildUrl("Y");
      var url = MorpheuzConfig.mConst().pushoverAPI;
      var msg = "token=" + potoken + "&user=" + pouser + "&message=" + encodeURIComponent(resetDate) + "&url=" + encodeURIComponent(urlToAttach) + "&url_title=Report" + "&priority=-2" + "&sound=none";
      console.log("pushoverTransmit: msg=" + msg);
      MorpheuzAjax.makePostAjaxCall(url, msg, function(resp) {
        console.log("pushoverTransmit: " + JSON.stringify(resp));
        if (resp.status !== 1) {
          MorpheuzUtil.setNoDef("postat", JSON.stringify(resp.errors));
        } else {
          MorpheuzUtil.setNoDef("postat", MorpheuzConfig.mLang().ok);
        }
      });
    } catch (err) {
      MorpheuzUtil.setNoDef("postat", err.message);
    }
  };

  module.exports = MorpheuzPushover;

}());
