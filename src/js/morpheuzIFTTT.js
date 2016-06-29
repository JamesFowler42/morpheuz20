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

  var MorpheuzIFTTT = {};

  /*
   * 
   */
  function getIfServer() {
    var ifserver = MorpheuzUtil.getWithDef("ifserver", "");
    var url;
    if (ifserver === "") {
      url = MorpheuzConfig.mConst().makerDefaultServer;
    } else {
      url = ifserver;
      if (ifserver.charAt(ifserver.length - 1) != '/') {
        url += '/';
      }
    }
    return url;
  }

  /*
   * Call the ifttt maker interface when the alarm sounds
   */
  MorpheuzIFTTT.iftttMakerInterfaceAlarm = function() {

    try {

      // Find out config information
      var ifkey = MorpheuzUtil.getWithDef("ifkey", "");

      // Escape if not configured
      if (ifkey === "") {
        console.log("ifttt maker deactivated");
        MorpheuzUtil.setNoDef("ifstat", MorpheuzConfig.mLang().disabled);
        return;
      }

      var payload = {
        "value1" : "",
        "value2" : "",
        "value3" : ""
      };

      var url = getIfServer() + MorpheuzConfig.mConst().makerAlarmUrl + ifkey;

      console.log("iftttMakerInterfaceAlarm: url=" + url);
      MorpheuzUtil.setNoDef("ifstat", MorpheuzConfig.mLang().sending);
      MorpheuzAjax.makeAjaxCall("POST", url, MorpheuzConfig.mConst().timeout, JSON.stringify(payload), function(resp) {
        console.log("iftttMakerInterfaceAlarm: " + JSON.stringify(resp));
        if (resp.status !== 1) {
          MorpheuzUtil.setNoDef("ifstat", JSON.stringify(resp.errors));
        } else {
          MorpheuzUtil.setNoDef("ifstat", MorpheuzConfig.mLang().ok);
        }
      });

    } catch (err) {
      MorpheuzUtil.setNoDef("ifstat", err.message);
    }
  };

  /*
   * Call the ifttt maker interface when the data export needs to be done
   */
  MorpheuzIFTTT.iftttMakerInterfaceData = function() {

    try {

      // Find out config information
      var ifkey = MorpheuzUtil.getWithDef("ifkey", "");

      // Escape if not configured
      if (ifkey === "") {
        console.log("ifttt maker deactivated");
        MorpheuzUtil.setNoDef("ifstat", MorpheuzConfig.mLang().disabled);
        return;
      }

      var base = parseInt(MorpheuzUtil.getNoDef("base"), 10);
      var resetDate = new Date(base).format(MorpheuzConfig.mConst().displayDateFmt);
      var urlToAttach = MorpheuzUtil.buildUrl("Y");

      var goneoff = MorpheuzUtil.getWithDef("goneOff", "N");
      var splitup = MorpheuzUtil.extractSplitup();
      var fromhr = MorpheuzUtil.getWithDef("fromhr", MorpheuzConfig.mConst().fromhrDef);
      var tohr = MorpheuzUtil.getWithDef("tohr", MorpheuzConfig.mConst().tohrDef);
      var frommin = MorpheuzUtil.getWithDef("frommin", MorpheuzConfig.mConst().fromminDef);
      var tomin = MorpheuzUtil.getWithDef("tomin", MorpheuzConfig.mConst().tominDef);
      var smartOn = MorpheuzUtil.getWithDef("smart", MorpheuzConfig.mConst().smartDef);
      var snoozes = MorpheuzUtil.getWithDef("snoozes", "0");

      var csvData = MorpheuzCommon.generateCopyLinkData(base, splitup, smartOn, fromhr, frommin, tohr, tomin, goneoff, snoozes);

      var payload = {
        "value1" : resetDate,
        "value2" : urlToAttach,
        "value3" : csvData.body
      };

      var url = getIfServer() + MorpheuzConfig.mConst().makerDataUrl + ifkey;

      console.log("iftttMakerInterfaceData: url=" + url);
      MorpheuzUtil.setNoDef("ifstat", MorpheuzConfig.mLang().sending);
      MorpheuzAjax.makeAjaxCall("POST", url, MorpheuzConfig.mConst().timeout, JSON.stringify(payload), function(resp) {
        console.log("iftttMakerInterfaceData: " + JSON.stringify(resp));
        if (resp.status !== 1) {
          MorpheuzUtil.setNoDef("ifstat", JSON.stringify(resp.errors));
        } else {
          MorpheuzUtil.setNoDef("ifstat", MorpheuzConfig.mLang().ok);
        }
      });

    } catch (err) {
      MorpheuzUtil.setNoDef("ifstat", err.message);
    }
  };

  /*
   * Call the ifttt maker interface when the bedtime is activated
   */
  MorpheuzIFTTT.iftttMakerInterfaceBedtime = function() {

    try {

      // Find out config information
      var ifkey = MorpheuzUtil.getWithDef("ifkey", "");

      // Escape if not configured
      if (ifkey === "") {
        console.log("ifttt maker deactivated");
        MorpheuzUtil.setNoDef("ifstat", MorpheuzConfig.mLang().disabled);
        return;
      }

      var payload = {
        "value1" : "",
        "value2" : "",
        "value3" : ""
      };

      var url = getIfServer() + MorpheuzConfig.mConst().makerBedtimeUrl + ifkey;

      console.log("iftttMakerInterfaceBedtime: url=" + url);
      MorpheuzUtil.setNoDef("ifstat", MorpheuzConfig.mLang().sending);
      MorpheuzAjax.makeAjaxCall("POST", url, MorpheuzConfig.mConst().timeout, JSON.stringify(payload), function(resp) {
        console.log("iftttMakerInterfaceBedtime: " + JSON.stringify(resp));
        if (resp.status !== 1) {
          MorpheuzUtil.setNoDef("ifstat", JSON.stringify(resp.errors));
        } else {
          MorpheuzUtil.setNoDef("ifstat", MorpheuzConfig.mLang().ok);
        }
      });

    } catch (err) {
      MorpheuzUtil.setNoDef("ifstat", err.message);
    }
  };

  module.exports = MorpheuzIFTTT;

}());
