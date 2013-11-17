/**
 * 
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

function resetInfo() {
	console.log("data reset");
	window.localStorage.clear();
}

function storePointInfo(point) {
	var day = window.localStorage.getItem("day");
	if (day == null) {
		var dayStr = new Date().format("ddMM");
		var hourStr = new Date().format("hh");
		var base = new Date().valueOf();
		console.log("Day not set - dayStr=" + dayStr + ", hourStr=" + hourStr + ", base=" + base);
		window.localStorage.setItem("day",new Date().format("ddMM"));
		window.localStorage.setItem("hour",new Date().format("hh"));
		window.localStorage.setItem("base", new Date().valueOf());
	} else {
		var today = new Date().format("ddMM");
		var yesterday = new Date().addDays(-1).format("ddMM");
		console.log("Day set - checking against today=" + today + ", yesterday=" + yesterday);
		if (day != today && day != yesterday)
			return;
   }
   var base = parseInt(window.localStorage.getItem("base"));
   var now = new Date().valueOf();
   var offset = Math.floor((now - base) / 900000);
   var entry = "P" + offset;
   console.log("Processing base=" + base + ", now=" + now + ", offset=" + offset + ", entry=" + entry);
   
   if (offset > 32) {
	   console.log("bailed out. Offset too big");
 	   return;
   }
 
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


Pebble.addEventListener("ready",
		function(e) {
	console.log("Ready");
	Pebble.sendAppMessage({"biggest": "-1"});
});

Pebble.addEventListener("appmessage",
		function(e) {
	console.log("Message Payload = " + e.payload.biggest);
	storePointInfo(parseInt(e.payload.biggest));
	Pebble.sendAppMessage({"biggest": "-1"});
});

Pebble.addEventListener("webviewclosed",
		function(e) {
	console.log("webview closed");
	console.log(e.type);
	var config = e.response;
	if (config == "reset")
		resetInfo();
});

Pebble.addEventListener("showConfiguration",
		function(e) {
	console.log("config");
	var day = window.localStorage.getItem("day");
	var hour = window.localStorage.getItem("hour");
	var graph = "";
	for (var i=0; i < 32; i++) {
		var entry = "P" + i;	
		var valueStr = window.localStorage.getItem(entry);
		if (valueStr == null) {
			graph = graph + "0!";
		} else {
			graph = graph + valueStr + "!";
		}
	}
	var url = "http://homepage.ntlworld.com/keith.j.fowler/morpheuz/view.html?day=" + day + "&hour=" + hour + "&graph=" + graph;
	console.log("url=" + url);
	Pebble.openURL(url);
});

Pebble.addEventListener("configurationClosed",
		function(e) {
	console.log("Configuration window returned: " + e.configurationData);
	if (e.configurationData == "reset")
		resetInfo();
});