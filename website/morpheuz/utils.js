/* 
 * Morpheuz Sleep Monitor
 *
 * Copyright (c) 2013-2015 James Fowler
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

/*
 * Constants
 */
function mUtil() {
  return {
    emailUrl : "json_email.php",
    emailToken : "morpheuz20",
    okResponse : "Sent OK",
    failResponse : "Failed to send with ",
    failGeneral : "Failed to send"
  };
}

function hrsmin(value) {
  var hours = Math.floor(value / 60);
  var minutes = value % 60;
  return fixLen(String(hours)) + ":" + fixLen(String(minutes));
}

/*
 * Some date functions
 */
Date.prototype.format = function(format) // author: meizz
{
  var monName = [ "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" ];
  var dayName = [ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" ];
  var o = {
    "M+" : this.getMonth() + 1, // month
    "d+" : this.getDate(), // day
    "h+" : this.getHours(), // hour
    "i+" : this.getHours() + 1, // hour + 1
    "m+" : this.getMinutes(), // minute
    "s+" : this.getSeconds(), // second
    "q+" : Math.floor((this.getMonth() + 3) / 3), // quarter
    "S" : this.getMilliseconds()
  // millisecond
  }

  if (/(y+)/.test(format)) {
    format = format.replace(RegExp.$1, (this.getFullYear() + "").substr(4 - RegExp.$1.length));
  }
  for ( var k in o) {
    if (new RegExp("(" + k + ")").test(format)) {
      format = format.replace(RegExp.$1, RegExp.$1.length == 1 ? o[k] : ("00" + o[k]).substr(("" + o[k]).length));
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

Date.prototype.addMinutes = function(minutes) {
  var copiedDate = new Date(this.getTime());
  return new Date(copiedDate.getTime() + minutes * 60000);
}

/*
 * Fix a string to 2 characters long prefixing a zero
 */
function fixLen(inStr) {
  if (inStr == null || inStr.length > 1)
    return inStr;
  return "0" + inStr;
}

/*
 * Extract parameters from URL
 */
function getParameterByName(name) {
  name = name.replace(/[\[]/, "\\\[").replace(/[\]]/, "\\\]");
  var regex = new RegExp("[\\?&]" + name + "=([^&#]*)"), results = regex.exec(location.search);
  return results == null ? "" : decodeURIComponent(results[1].replace(/\+/g, " "));
}

/*
 * Set the on screen version warning text if the version is non-current
 */
function setScreenMessageBasedOnVersion(vers) {
  $(".versproblem").show();
  $.ajaxSetup({ scriptCharset: "utf-8" , contentType: "application/json; charset=utf-8"});
  $.getJSON("currentversion.json?v=" + new Date().getTime(), function(data) {
    if (typeof data !== "undefined" && typeof data.version !== "undefined") {
      var currentVer = parseInt(data.version, 10);
      var requestVer = parseInt(vers, 10);
      $(".versproblem").hide();
      if (currentVer > requestVer) {
        $(".verswarning").show();
      } else if (currentVer < requestVer) {
        $(".versbeta").show();
      }
    }
  }).error(function(args) {
    $(".versproblem").text("Error attempting to find the current version: " + JSON.stringify(args));
  });
}

/*
 * Adjust for viewport
 */
function scaleToViewport() {

  // Get the width and limit to 320px to 800px
  var viewportWidth = $(window).width();
  if (viewportWidth > 800) {
    viewportWidth = 800;
  } else if (viewportWidth < 320) {
    viewportWidth = 320;
  }
  viewportWidth -= 5;

  // Set any element that has requested it to the required width
  $(".vpwidth").width(viewportWidth);
  $(".vpheight").height(viewportWidth * 250 / 318);

  // Ask the charts to replot
  if (typeof document.plot1 !== "undefined") {
    document.plot1.replot();
  }
  if (typeof document.plot2 !== "undefined") {
    document.plot2.replot();
  }
}

/*
 * Setup to auto adjust for viewport
 */
function adjustForViewport() {
  scaleToViewport();
  $(window).resize(scaleToViewport);
}

/*
 * Send an email via the server php
 */
function sendMailViaServer(email, resp) {
  try {
    var msg = "email=" + encodeURIComponent(JSON.stringify(email));

    var req = new XMLHttpRequest();
    req.open("POST", mUtil().emailUrl, true);
    req.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
    req.setRequestHeader("X-Client-token", mUtil().emailToken);
    req.onreadystatechange = function(e) {
      if (req.readyState == 4) {
        if (req.status == 200) {
          resp(1, mUtil().okResponse);
        } else {
          resp(0, mUtil().failResponse + req.status);
        }
      }
    }
    req.onerror = function(ex) {
      resp(0, mUtil().failGeneral);
    }
    req.send(msg);
  } catch (err) {
    resp(0, mUtil().failResponse + err.message);
  }
}

/*
 * Validate email address
 */
function validateEmail(email) 
{
    var re = /[^\s@]+@[^\s@]+\.[^\s@]+/;
    return re.test(email);
}

/*
 * Trim safely
 */
function safeTrim(strval) {
  try {
    return strval.trim();
  } catch (err) {
    return strval;
  }
}
