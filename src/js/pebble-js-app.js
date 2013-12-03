/*
 * Constants
 */
function mConst() {
	var cfg = { version : "v0.8",
			    limit: 54,
			    divisor: 600000,
			    html: "view4.html" };
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
	console.log("data reset");
	window.localStorage.clear();
}

/*
 * Store data returned from the watch
 */
function storePointInfo(point) {
	var day = window.localStorage.getItem("day");
	if (day == null) {
		var dayStr = new Date().format("ddMM");
		var base = new Date().valueOf();
		console.log("Day not set - dayStr=" + dayStr + ", base=" + base);
		window.localStorage.setItem("day",dayStr);
		window.localStorage.setItem("base", base);
	} else {
		var today = new Date().format("ddMM");
		var yesterday = new Date().addDays(-1).format("ddMM");
		console.log("Day set - checking against today=" + today + ", yesterday=" + yesterday);
		if (day != today && day != yesterday) {
			return;
		}
   }
   var base = parseInt(window.localStorage.getItem("base"));
   var now = new Date().valueOf();
   var offset = Math.floor((now - base) / mConst().divisor);
   var entry = "P" + offset;
   console.log("Processing base=" + base + ", now=" + now + ", offset=" + offset + ", entry=" + entry);
   
   if (offset > mConst().limit) {
	   console.log("bailed out. Offset too big");
 	   return;
   }
 
   // Now store entries
   var valueStr = window.localStorage.getItem(entry);
   if (valueStr == null) {
	   window.localStorage.setItem(entry,point);
	   console.log(entry + " = " + point);
   } else {
	   var value = parseInt(valueStr);
	   if (point > value) {
		   window.localStorage.setItem(entry,point);
		   console.log(entry + " = " + point);
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
   console.log("threshold=" + threshold);
	
	// Are we in the right timeframe
	var fromhr = window.localStorage.getItem("fromhr");
	var tohr = window.localStorage.getItem("tohr");
	var frommin = window.localStorage.getItem("frommin");
	var tomin = window.localStorage.getItem("tomin");

	var from = fromhr + frommin;
	var to = tohr + tomin;
	
	var now = new Date().format("hhmm");

	console.log("from=" + from + ", to=" + to + ", now=" + now);
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
	console.log("Ready");
	Pebble.sendAppMessage({"biggest": "-1", "alarm": 0});
});

/*
 * Process sample from the watch
 */
Pebble.addEventListener("appmessage",
		function(e) {
	console.log("Message Payload = " + e.payload.biggest);
	var point = parseInt(e.payload.biggest);
	storePointInfo(point);
    var alarm = smart_alarm(point);
	Pebble.sendAppMessage({"biggest": "-1", "alarm": alarm});
});

/*
 * Monitor the closing of the config/display screen so as we can do a reset if needed
 */
Pebble.addEventListener("webviewclosed",
		function(e) {
	console.log("webview closed");
	console.log(e.type);
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
	}
});

/*
 * Show the config/display page - this will show a graph and allow a reset
 */
Pebble.addEventListener("showConfiguration",
		function(e) {
	console.log("config");
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
			
	var url = "http://homepage.ntlworld.com/keith.j.fowler/morpheuz/" + mConst().html + "?base=" + base + "&graph=" + graph + 
		      "&fromhr=" + fromhr + "&tohr=" + tohr + "&frommin=" + frommin + "&tomin=" + tomin + "&smart=" + smart + "&vers=" + mConst().version;
	console.log("url=" + url);
	Pebble.openURL(url);
});

/*
 * Unclear if this serves a purpose at all
 */
Pebble.addEventListener("configurationClosed",
		function(e) {
	console.log("Configuration window returned: " + e.configurationData);
});