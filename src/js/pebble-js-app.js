/* 
 * Morpheuz Sleep Monitor
 *
 * Copyright (c) 2013 James Fowler
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
	var cfg = { version : "v1.1",
			limit: 54,
			divisor: 600000,
			url: "http://homepage.ntlworld.com/keith.j.fowler/morpheuz/view5.html" };
	return cfg;
}
/**
 * Various date functions
 */
Date.prototype.format = function(format) //author: meizz
{
	var o = {
			"M+" : this.getMonth()+1, //month
			"d+" : this.getDate(),    //day
			"h+" : this.getHours(),   //hour
			"i+" : this.getHours() + 1,   //hour + 1
			"m+" : this.getMinutes(), //minute
			"s+" : this.getSeconds(), //second
			"q+" : Math.floor((this.getMonth()+3)/3),  //quarter
			"S" : this.getMilliseconds() //millisecond
	}

	if(/(y+)/.test(format)) format=format.replace(RegExp.$1,
			(this.getFullYear()+"").substr(4 - RegExp.$1.length));
	for(var k in o)if(new RegExp("("+ k +")").test(format))
		format = format.replace(RegExp.$1,
				RegExp.$1.length==1 ? o[k] :
					("00"+ o[k]).substr((""+ o[k]).length));
	return format;
}

Date.prototype.addDays = function (num) {
	var value = this.valueOf();
	value += 86400000 * num;
	return new Date(value);
}

Date.prototype.addSeconds = function (num) {
	var value = this.valueOf();
	value += 1000 * num;
	return new Date(value);
}

Date.prototype.addMinutes = function (num) {
	var value = this.valueOf();
	value += 60000 * num;
	return new Date(value);
}

Date.prototype.addHours = function (num) {
	var value = this.valueOf();
	value += 3600000 * num;
	return new Date(value);
}

/*
 * Reset log
 */
function resetInfo() {
	console.log("reset");
	window.localStorage.clear();
	var dayStr = new Date().format("ddMM");
	var base = new Date().valueOf();
	window.localStorage.setItem("day",dayStr);
	window.localStorage.setItem("base", base);
}

/*
 * Store data returned from the watch
 */
function storePointInfo(point) {

	// Wrong day filter
	var day = window.localStorage.getItem("day");
	var today = new Date().format("ddMM");
	var yesterday = new Date().addDays(-1).format("ddMM");
	if (day != today && day != yesterday) {
		return;
	}

	// Locate correct entry
	var base = parseInt(window.localStorage.getItem("base"));
	var now = new Date().valueOf();
	var offset = Math.floor((now - base) / mConst().divisor);
	var entry = "P" + offset;

	if (offset > mConst().limit) {
		return;
	}

	// Now store entries
	var valueStr = window.localStorage.getItem(entry);
	if (valueStr == null) {
		window.localStorage.setItem(entry,point);
	} else {
		var value = parseInt(valueStr);
		if (point > value) {
			window.localStorage.setItem(entry,point);
		}
	}
}

/*
 * Perform smart alarm function
 */
function smart_alarm(point) {

	// Are we doing smart alarm thing
	var smart = window.localStorage.getItem("smart");
	if (smart == null || smart != 'Y')
		return 0;

	// Now has the alarm been sounded yet
	var goneOff = window.localStorage.getItem("goneOff");
	if (goneOff != null)
		return 0;

	// Work out the average
	var total = 0;
	var novals = 0;
	for (var i=0; i < mConst().limit; i++) {
		var entry = "P" + i;	
		var valueStr = window.localStorage.getItem(entry);
		if (valueStr != null) {
			novals++;
			total = total + parseInt(valueStr);
		} 
	}
	if (novals == 0)
		novals = 1;
	var threshold = total / novals;	

	// Are we in the right timeframe
	var fromhr = window.localStorage.getItem("fromhr");
	var tohr = window.localStorage.getItem("tohr");
	var frommin = window.localStorage.getItem("frommin");
	var tomin = window.localStorage.getItem("tomin");

	var from = fromhr + frommin;
	var to = tohr + tomin;

	var now = new Date().format("hhmm");

	if (now >= from && now < to) {

		// Has the current point exceeded the threshold value
		if (point > threshold) {
			window.localStorage.setItem("goneOff","Y");
			return 1;
		} else {
			return 0;
		}
	}

	var before = new Date().addMinutes(-1).format("hhmm");
	var after = new Date().addMinutes(1).format("hhmm");
	// Or failing that have we hit the last minute we can
	if (now == to || before == to || after == to) { 
		window.localStorage.setItem("goneOff","Y");
		return 1;
	}

	// None of the above
	return 0;
}

/*
 * Process ready from the watch
 */
Pebble.addEventListener("ready",
		function(e) {
	console.log("ready");
	var smartStr = window.localStorage.getItem("smart");
	if (smartStr == null) {
		resetInfo();
		window.localStorage.setItem("smart","N");
		window.localStorage.setItem("fromhr","6");
		window.localStorage.setItem("frommin","30");
		window.localStorage.setItem("tohr","7");
		window.localStorage.setItem("tomin","15");
	}		
	Pebble.sendAppMessage(returnSmartAlarmSettings());
});

/*
 * Process sample from the watch
 */
Pebble.addEventListener("appmessage",
		function(e) {
	var point = parseInt(e.payload.biggest);
	console.log("appmessage biggest=" + point);
	storePointInfo(point);
	var alarm = smart_alarm(point);
	if (alarm == 1) {
		// Only reply to fire the alarm - no reply otherwise
		Pebble.sendAppMessage({"alarm": 1});
	}
});

/*
 * Return smart alarm setting to watchface
 */
function returnSmartAlarmSettings() {
	var smartStr = window.localStorage.getItem("smart");
	if (smartStr != null && smartStr == "Y") {
		var fromhr = parseInt(window.localStorage.getItem("fromhr"));
		var tohr = parseInt(window.localStorage.getItem("tohr"));
		var frommin = parseInt(window.localStorage.getItem("frommin"));
		var tomin = parseInt(window.localStorage.getItem("tomin"));
		var from = (fromhr << 8) | frommin;
		var to = (tohr << 8) | tomin;
		return {"from": from,
			"to": to};
	} else {
		return {"from": -1,
			"to": -1};
	}
}

/*
 * Monitor the closing of the config/display screen so as we can do a reset if needed
 */
Pebble.addEventListener("webviewclosed",
		function(e) {
	console.log("webviewclosed");
	if (e.response == null)
		return;
	var dataElems = e.response.split("!");
	if (dataElems[0] == "reset") {
		resetInfo();
		window.localStorage.setItem("smart",dataElems[1]);
		window.localStorage.setItem("fromhr",dataElems[2]);
		window.localStorage.setItem("frommin",dataElems[3]);
		window.localStorage.setItem("tohr",dataElems[4]);
		window.localStorage.setItem("tomin",dataElems[5]);
		Pebble.sendAppMessage(returnSmartAlarmSettings());
	}
});

/*
 * Show the config/display page - this will show a graph and allow a reset
 */
Pebble.addEventListener("showConfiguration",
		function(e) {
	var base = window.localStorage.getItem("base");
	var graph = "";
	for (var i=0; i < mConst().limit; i++) {
		var entry = "P" + i;	
		var valueStr = window.localStorage.getItem(entry);
		if (valueStr == null) {
			graph = graph + "-1!";
		} else {
			graph = graph + valueStr + "!";
		}
	}
	var fromhr = window.localStorage.getItem("fromhr");
	var tohr = window.localStorage.getItem("tohr");
	var frommin = window.localStorage.getItem("frommin");
	var tomin = window.localStorage.getItem("tomin");
	var smart = window.localStorage.getItem("smart");

	var url = mConst().url + "?base=" + base + "&graph=" + graph + 
	"&fromhr=" + fromhr + "&tohr=" + tohr + "&frommin=" + frommin + "&tomin=" + tomin + "&smart=" + smart + "&vers=" + mConst().version;
	console.log("url=" + url);
	Pebble.openURL(url);
});

/*
 * Unclear if this serves a purpose at all
 */
Pebble.addEventListener("configurationClosed",
		function(e) {
	console.log("configurationClosed");
});