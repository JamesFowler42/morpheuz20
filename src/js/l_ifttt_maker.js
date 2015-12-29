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

/*global nvl, window, mLang, makeAjaxCall, mConst, buildUrl, generateCopyLinkData, extractSplitup */
/*exported iftttMakerInterfaceAlarm, iftttMakerInterfaceData, iftttMakerInterfaceBedtime */

/*
 *
 */
function getIfServer() {
  var ifserver = nvl(window.localStorage.getItem("ifserver"), "");
  var url;
  if (ifserver === "") {
    url = mConst().makerDefaultServer;
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
function iftttMakerInterfaceAlarm() {

  try {
  
    // Find out config information
    var ifkey =  nvl(window.localStorage.getItem("ifkey"), "");
  
    // Escape if not configured
    if (ifkey === "") {
      console.log("ifttt maker deactivated");
      window.localStorage.setItem("ifstat", mLang().disabled);
      return;
    }
  
    var payload = { "value1" : "", "value2" : "", "value3" : "" };

    var url = getIfServer() + mConst().makerAlarmUrl + ifkey;
    
    console.log("iftttMakerInterfaceAlarm: url=" + url);
    window.localStorage.setItem("ifstat", mLang().sending);
    makeAjaxCall("POST", url, mConst().timeout, JSON.stringify(payload), function(resp) {
      console.log("iftttMakerInterfaceAlarm: " + JSON.stringify(resp));
      if (resp.status !== 1) {
        window.localStorage.setItem("ifstat", JSON.stringify(resp.errors));
      } else {
        window.localStorage.setItem("ifstat", mLang().ok);
      }     
    });
    
  } catch (err) {
    window.localStorage.setItem("ifstat", err.message);
  }
}

/*
 * Call the ifttt maker interface when the data export needs to be done
 */
function iftttMakerInterfaceData() {

  try {
  
    // Find out config information
    var ifkey =  nvl(window.localStorage.getItem("ifkey"), "");
  
    // Escape if not configured
    if (ifkey === "") {
      console.log("ifttt maker deactivated");
      window.localStorage.setItem("ifstat", mLang().disabled);
      return;
    }
   
    var base = parseInt(window.localStorage.getItem("base"), 10);
    var resetDate = new Date(base).format(mConst().displayDateFmt);
    var urlToAttach = buildUrl("Y");

    var goneoff = nvl(window.localStorage.getItem("goneOff"), "N");
    var splitup = extractSplitup();
    var fromhr = nvl(window.localStorage.getItem("fromhr"), mConst().fromhrDef);
    var tohr = nvl(window.localStorage.getItem("tohr"), mConst().tohrDef);
    var frommin = nvl(window.localStorage.getItem("frommin"), mConst().fromminDef);
    var tomin = nvl(window.localStorage.getItem("tomin"), mConst().tominDef);
    var smartOn = nvl(window.localStorage.getItem("smart"), mConst().smartDef);
    
    var csvData = generateCopyLinkData(base, splitup, smartOn, fromhr, frommin, tohr, tomin, goneoff);
  
    var payload = { "value1" : resetDate, "value2" : urlToAttach, "value3" : csvData.body };

    var url = getIfServer() + mConst().makerDataUrl + ifkey;
    
    console.log("iftttMakerInterfaceData: url=" + url);
    window.localStorage.setItem("ifstat", mLang().sending);
    makeAjaxCall("POST", url, mConst().timeout, JSON.stringify(payload), function(resp) {
      console.log("iftttMakerInterfaceData: " + JSON.stringify(resp));
      if (resp.status !== 1) {
        window.localStorage.setItem("ifstat", JSON.stringify(resp.errors));
      } else {
        window.localStorage.setItem("ifstat", mLang().ok);
      }     
    });
    
  } catch (err) {
    window.localStorage.setItem("ifstat", err.message);
  }
}

/*
 * Call the ifttt maker interface when the bedtime is activated
 */
function iftttMakerInterfaceBedtime() {

  try {
  
    // Find out config information
    var ifkey =  nvl(window.localStorage.getItem("ifkey"), "");
  
    // Escape if not configured
    if (ifkey === "") {
      console.log("ifttt maker deactivated");
      window.localStorage.setItem("ifstat", mLang().disabled);
      return;
    }
  
    var payload = { "value1" : "", "value2" : "", "value3" : "" };

    var url = getIfServer() + mConst().makerBedtimeUrl + ifkey;
    
    console.log("iftttMakerInterfaceBedtime: url=" + url);
    window.localStorage.setItem("ifstat", mLang().sending);
    makeAjaxCall("POST", url, mConst().timeout, JSON.stringify(payload), function(resp) {
      console.log("iftttMakerInterfaceBedtime: " + JSON.stringify(resp));
      if (resp.status !== 1) {
        window.localStorage.setItem("ifstat", JSON.stringify(resp.errors));
      } else {
        window.localStorage.setItem("ifstat", mLang().ok);
      }     
    });
    
  } catch (err) {
    window.localStorage.setItem("ifstat", err.message);
  }
}

