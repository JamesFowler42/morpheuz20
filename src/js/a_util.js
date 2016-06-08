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

/*exported getPlatform, toHexStr, getWithDef, getNoDef, setNoDef, setWithDef */

/*global nvl, window */

/*
 * Which watch platform
 */
function getPlatform() {
  try {
    var wi = Pebble.getActiveWatchInfo();
    if (wi && wi.platform) {
      return wi.platform;
    } else {
      return "unknown";
    }
  } catch (err) {
    return "unknown";
  }
}

/*
 * Convert to fixed length hex string
 */
function toHexStr(number, length, maxValueCap, minValueCap) {
  var num = parseInt(number, 10);
  if (num > maxValueCap) {
    num = maxValueCap;
  } else if (num < minValueCap) {
    num = minValueCap;
  }
  var str = '' + num.toString(16);
  while (str.length < length)
    str = '0' + str;
  return str;
}

/*
 * Often used pick a local stored value with a default
 */
function getWithDef(vName, defVal) {
  return nvl(getNoDef(vName), defVal);
}

/*
 * Get local storage without a default
 */
function getNoDef(vName) {
  //console.log("getItem(" + vName + ")");
  return window.localStorage.getItem(vName);
}

/*
 * Set local storage without a default
 */
function setNoDef(vName, vValue) {
  //console.log("setItem(" + vName + "," + vValue + ")");
  window.localStorage.setItem(vName, vValue);
}

/*
 * Set local storage with a default
 */
function setWithDef(vName, vValue, defVal) {
  setNoDef(vName, nvl(vValue, defVal));
}