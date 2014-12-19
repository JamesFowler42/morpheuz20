/* 
 * Morpheuz Sleep Monitor
 *
 * Copyright (c) 2013-2014 James Fowler
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

/*global XMLHttpRequest, window, console*/

/*
 * Some date functions
 */
Date.prototype.format = function(format) // author: meizz
{
  var monName = ["Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"];
  var dayName = ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"];
  var o = {
    "M+" : this.getMonth() + 1, // month
    "d+" : this.getDate(), // day
    "h+" : this.getHours(), // hour
    "i+" : this.getHours() + 1, // hour + 1
    "m+" : this.getMinutes(), // minute
    "s+" : this.getSeconds(), // second
    "q+" : Math.floor((this.getMonth() + 3) / 3), // quarter
    "S" : this.getMilliseconds()  // millisecond
  }

  if (/(y+)/.test(format)) {
    format = format.replace(RegExp.$1, (this.getFullYear() + "")
        .substr(4 - RegExp.$1.length));
  }
  for ( var k in o) {
    if (new RegExp("(" + k + ")").test(format)) {
      format = format.replace(RegExp.$1, RegExp.$1.length == 1 ? o[k]
          : ("00" + o[k]).substr(("" + o[k]).length));
    }
  }
  if (/(N+)/.test(format)) {
     format = format.replace(RegExp.$1, monName[this.getMonth()]);
  } 
  if (/(W+)/.test(format)) {
    format = format.replace(RegExp.$1, dayName[this.getDay()]);
  }
  return format;
}

/*
 * Add minutes to date
 */
Date.prototype.addMinutes = function(minutes) {
  var copiedDate = new Date(this.getTime());
  return new Date(copiedDate.getTime() + minutes * 60000);
}

/*
 * Protection against nulls
 */
function nvl(field, defval) {
  return field === null || field === "null" || typeof field === "undefined" ? defval : field;
}


/*
 * Fix a string to 2 characters long prefixing a zero
 */
function fixLen(inStr) {
  if (inStr === null || inStr.length > 1)
    return inStr;
  return "0" + inStr;
}

