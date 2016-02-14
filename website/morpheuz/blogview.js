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

/*
 * Constants
 */
function mConst() {
	var cfg = {
		url : "http://ui.morpheuz.net/morpheuz/view-",
		currentVersUrl : "http://ui.morpheuz.net/morpheuz/currentversion.json?v=",
		awakeAbove : 1000,
		lightAbove : 120,
		sampleIntervalMins : 10
	};
	return cfg;
}

/*
 * Fix a string to 2 characters long prefixing a zero
 */
function fixLen(inStr) {
	if (inStr == null || inStr.length > 1)
		return inStr;
	return '0' + inStr;
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

	if (/(y+)/.test(format))
		format = format.replace(RegExp.$1, (this.getFullYear() + "")
				.substr(4 - RegExp.$1.length));
	for ( var k in o)
		if (new RegExp("(" + k + ")").test(format))
			format = format.replace(RegExp.$1, RegExp.$1.length == 1 ? o[k]
					: ("00" + o[k]).substr(("" + o[k]).length));
	return format;
}

Date.prototype.addMinutes = function(minutes) {
	var copiedDate = new Date(this.getTime());
	return new Date(copiedDate.getTime() + minutes * 60000);
}

function writeError(error) {
	document.getElementById("error").textContent = error;
}

function getHrFromDate(dateStr) {
	return dateStr.substr(0, 2);
}

function getMinFromDate(dateStr) {
	return dateStr.substr(3, 2);
}

document.getElementById('clear').onclick = function() {
	writeError("");
	document.getElementById('output').style.display = 'none';
	document.getElementById("data").value = "";
}

/*
 * Get the version of the file we need
 */
function getVersion(result) {
  $.ajaxSetup({ scriptCharset: "utf-8" , contentType: "application/json; charset=utf-8", async: false});
  $.getJSON(mConst().currentVersUrl + new Date().getTime(), function(data) {
    if (typeof data !== "undefined" && typeof data.version !== "undefined") {
      var currentVer = parseInt(data.version, 10);
	  result(currentVer);
    }
  }).error(function(args) {
    writeError("Error attempting to find the current version: " + JSON.stringify(args));
  });
}

/*
 * Build the url for the config and report display @param noset
 */
function buildUrl(version, base, fromhr, tohr, frommin, tomin, smart, goneOff, graphIn) {
  var graph = "";
  for (var i = 0; i < graphIn.length; i++) {
	  graph += graphIn[i] + "!";
  }
  
  var url = mConst().url + version + ".html" + 
           "?base=" + base + "&graph=" + graph + "&fromhr=" + fromhr + "&tohr=" + tohr + "&frommin=" + frommin + "&tomin=" + tomin + 
           "&smart=" + smart + "&vers=" + version + "&goneoff=" + goneOff + "&emailto=" +
           "&noset=Y";
  
  return url;
}

$('#output').load(function() {
    this.style.height =
    (this.contentWindow.document.body.offsetHeight + 5) + 'px';
});

document.getElementById('plot').onclick = function() {

	writeError("");
	document.getElementById('output').style.display = 'none';

	var data = document.getElementById("data").value;
	if (data == null || data == "") {
		writeError("Please paste some data into the text area above and press 'Plot' again");
		return;
	}

	var breakup = data.split("\n");
	var lns = 0;
	for (var i = 0; i < breakup.length; i++) {
		if (breakup[i] != null && breakup[i].trim() !== '')
			lns++;
	}

	//if (lns != 54 && lns != 57 && lns != 56) {
	//	writeError("There must be 54, 56 or 57 lines pasted into the text area");
	//	return;
	//}

	var base = new Date().valueOf();
	var graph = new Array();

	var fromhr = "06";
	var frommin = "15";
	var tohr = "08";
	var tomin = "00";
	var smart = "N";
	var goneoff = "N";

	for (var i = 0; i < breakup.length; i++) {
		if (breakup[i] == null || breakup[i].trim() === '')
			continue;
		var lineSplit = breakup[i].split(",");
		if (lineSplit.length != 2) {
			writeError("Each line must be a comma separated pair of values");
			return;
		}
		var timeComp = lineSplit[0].trim();
		var valueComp = lineSplit[1].trim().toUpperCase();
		if (i == 0) {
			var baseStr = new Date().format('yyyy/MM/dd') + ' ' + timeComp
					+ ":00";
			var baseDt = new Date(baseStr);
			base = baseDt.valueOf();
		}
		if (valueComp == 'START') {
			fromhr = getHrFromDate(timeComp);
			frommin = getMinFromDate(timeComp);
			smart = 'Y';
		} else if (valueComp == 'END') {
			tohr = getHrFromDate(timeComp);
			tomin = getMinFromDate(timeComp);
			smart = 'Y';
		} else if (valueComp == 'ALARM') {
			goneoff = getHrFromDate(timeComp) + getMinFromDate(timeComp);
		} else {
			graph[i] = valueComp;
		}
	}
	
	getVersion(function(version) {
		var url = buildUrl(version, base, fromhr, tohr, frommin, tomin, smart, goneoff, graph);
		$("#output").attr("src", url).show();
	});

	
	
	
}
