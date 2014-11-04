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
		limit : 54,
		divisor : 600000,
		url : "http://homepage.ntlworld.com/keith.j.fowler/morpheuz/view-",
		ctrlReset : 1,
		versionDef : "22",
		smartDef : "N",
		fromhrDef : "6",
		fromminDef : "30",
		tohrDef : "7",
		tominDef : "15"
	};
	return cfg;
}

/*
 * Protection against nulls
 */
function nvl(field, defval) {
	return field == null || field == "null" ? defval : field;
}

/*
 * Reset log
 */
function resetWithPreserve() {
	console.log("resetWithPreserve");
	var version = window.localStorage.getItem("version");
	var fromhr = window.localStorage.getItem("fromhr");
	var tohr = window.localStorage.getItem("tohr");
	var frommin = window.localStorage.getItem("frommin");
	var tomin = window.localStorage.getItem("tomin");
	var smart = window.localStorage.getItem("smart");
	var emailto = window.localStorage.getItem("emailto");
	var xuser = window.localStorage.getItem("xuser");
	var xpass = window.localStorage.getItem("xpass");
	var xkey = window.localStorage.getItem("xkey");
	window.localStorage.clear();
	window.localStorage.setItem("version", nvl(version, mConst().versionDef));
	window.localStorage.setItem("smart", nvl(smart, mConst().smartDef));
	window.localStorage.setItem("fromhr", nvl(fromhr, mConst().fromhrDef));
	window.localStorage.setItem("frommin", nvl(frommin, mConst().fromminDef));
	window.localStorage.setItem("tohr", nvl(tohr, mConst().tohrDef));
	window.localStorage.setItem("tomin", nvl(tomin, mConst().tominDef));
	window.localStorage.setItem("emailto", nvl(emailto, ""));
	window.localStorage.setItem("xuser", nvl(xuser, ""));
	window.localStorage.setItem("xpass", nvl(xpass, ""));
	window.localStorage.setItem("xkey", nvl(xkey, ""));
}

/*
 * Store data returned from the watch
 */
function storePointInfo(point, biggest) {
	var entry = "P" + point;
	if (biggest == 0) // Don't pass -1 across the link but 0 really means null
		biggest = -1; // Null
	else if (biggest == 5000)
		biggest = -2; // Ignored by user
	window.localStorage.setItem(entry, biggest);
}

/*
 * Process ready from the watch
 */
Pebble.addEventListener("ready", function(e) {
	console.log("ready");
	var smartStr = window.localStorage.getItem("smart");
	if (smartStr == null || smartStr == "null") {
		resetWithPreserve();
		window.localStorage.setItem("smart", mConst().smartDef);
		window.localStorage.setItem("fromhr", mConst().fromhrDef);
		window.localStorage.setItem("frommin", mConst().fromminDef);
		window.localStorage.setItem("tohr", mConst().tohrDef);
		window.localStorage.setItem("tomin", mConst().tominDef);
		Pebble.sendAppMessage(returnSmartAlarmSettings(true));
	} else {
		Pebble.sendAppMessage(returnSmartAlarmSettings(false));
	}
});

/*
 * Process sample from the watch
 */
Pebble.addEventListener("appmessage", function(e) {
	if (typeof e.payload.keyBase !== "undefined") {
		var base = parseInt(e.payload.keyBase, 10);
		// Watch delivers local time in seconds...
		base = (base + (new Date().getTimezoneOffset() * 60)) * 1000;
		console.log("appmessage base=" + base);
		resetWithPreserve();
		window.localStorage.setItem("base", base);
	}
	if (typeof e.payload.keyVersion !== "undefined") {
		var version = parseInt(e.payload.keyVersion, 10);
		console.log("appmessage version=" + version);
		window.localStorage.setItem("version", version);
	}
	if (typeof e.payload.keyGoneoff !== "undefined") {
		var goneoffNum = parseInt(e.payload.keyGoneoff, 10);
		var goneoff = "N";
		if (goneoffNum != 0) {
			var hours = Math.floor(goneoffNum / 60);
			var minutes = goneoffNum - hours * 60;
			var hoursStr = String(hours);
			var minutesStr = String(minutes);
			if (hoursStr.length < 2)
				hoursStr = "0" + hoursStr;
			if (minutesStr.length < 2)
				minutesStr = "0" + minutesStr;
			goneoff = hoursStr + minutesStr;
		}
		console.log("appmessage goneoff=" + goneoff);
		window.localStorage.setItem("goneOff", goneoff);
	}
	if (typeof e.payload.keyPoint !== "undefined") {
		var point = parseInt(e.payload.keyPoint, 10);
		var top = point >> 16;
		var bottom = point & 0xFFFF;
		console.log("appmessage point=" + top + ", biggest=" + bottom);
		storePointInfo(top, bottom);
	}
});

/*
 * Return smart alarm setting to watchface
 */
function returnSmartAlarmSettings(resetVal) {
	var ctrlValue = 0;
	ctrlValue = resetVal ? ctrlValue + mConst().ctrlReset : ctrlValue;
	var smartStr = window.localStorage.getItem("smart");
	if (smartStr != null && smartStr == "Y") {
		var fromhr = parseInt(window.localStorage.getItem("fromhr"), 10);
		var tohr = parseInt(window.localStorage.getItem("tohr"), 10);
		var frommin = parseInt(window.localStorage.getItem("frommin"), 10);
		var tomin = parseInt(window.localStorage.getItem("tomin"), 10);
		var from = (fromhr << 8) | frommin;
		var to = (tohr << 8) | tomin;
		return {
			"keyFrom" : from,
			"keyTo" : to,
			"keyCtrl" : ctrlValue
		};
	} else {
		return {
			"keyFrom" : -1,
			"keyTo" : -1,
			"keyCtrl" : ctrlValue
		};
	}
}

/*
 * Monitor the closing of the config/display screen so as we can do a reset if
 * needed
 */
Pebble.addEventListener("webviewclosed", function(e) {
	console.log("webviewclosed " + e.response);
	if (e.response == null)
		return;
	var dataElems = e.response.split("!");
	if (dataElems[0] == "reset") {
		resetWithPreserve();
		window.localStorage.setItem("smart", dataElems[1]);
		window.localStorage.setItem("fromhr", dataElems[2]);
		window.localStorage.setItem("frommin", dataElems[3]);
		window.localStorage.setItem("tohr", dataElems[4]);
		window.localStorage.setItem("tomin", dataElems[5]);
		window.localStorage.setItem("emailto", dataElems[7]);
		window.localStorage.setItem("xuser", dataElems[8]);
		window.localStorage.setItem("xpass", dataElems[9]);
		window.localStorage.setItem("xkey", dataElems[10]);
		Pebble.sendAppMessage(returnSmartAlarmSettings(true));
	}
});

/*
 * Show the config/display page - this will show a graph and allow a reset
 */
Pebble.addEventListener("showConfiguration",
		function(e) {
			var base = window.localStorage.getItem("base");
			var graph = "";
			for (var i = 0; i < mConst().limit; i++) {
				var entry = "P" + i;
				var valueStr = window.localStorage.getItem(entry);
				if (valueStr == null) {
					graph = graph + "-1!";
				} else {
					graph = graph + valueStr + "!";
				}
			}
			var version = window.localStorage.getItem("version");
			if (!(parseInt(version, 10) >= 22))
				version = "22";

			var fromhr = nvl(window.localStorage.getItem("fromhr"),
					mConst().fromhrDef);
			var tohr = nvl(window.localStorage.getItem("tohr"),
					mConst().tohrDef);
			var frommin = nvl(window.localStorage.getItem("frommin"),
					mConst().fromminDef);
			var tomin = nvl(window.localStorage.getItem("tomin"),
					mConst().tominDef);
			var smart = nvl(window.localStorage.getItem("smart"),
					mConst().smartDef);
			var goneOff = nvl(window.localStorage.getItem("goneOff"), "N");
			var emailto = nvl(window.localStorage.getItem("emailto"), "");
			var xuser = nvl(window.localStorage.getItem("xuser"), "");
			var xpass = nvl(window.localStorage.getItem("xpass"), "");
			var xkey = nvl(window.localStorage.getItem("xkey"), "");

			var url = mConst().url + version + ".html?base=" + base + "&graph="
					+ graph + "&fromhr=" + fromhr + "&tohr=" + tohr
					+ "&frommin=" + frommin + "&tomin=" + tomin + "&smart="
					+ smart + "&vers=" + version + "&goneoff=" + goneOff
					+ "&emailto=" + emailto + "&xuser=" + xuser + "&xpass="
					+ xpass + "&xkey=" + xkey;
			console.log("url=" + url + " (len=" + url.length + ")");
			Pebble.openURL(url);
		});

/*
 * Unclear if this serves a purpose at all
 */
Pebble.addEventListener("configurationClosed", function(e) {
	console.log("configurationClosed");
});