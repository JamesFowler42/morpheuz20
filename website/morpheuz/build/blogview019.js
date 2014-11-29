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
	var cfg = {
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
	document.getElementById("error").textContent=error;
}

function getHrFromDate(dateStr) {
	return dateStr.substr(0,2);
}

function getMinFromDate(dateStr) {
	return dateStr.substr(3,2);
}

document.getElementById('clear').onclick = function() {
	writeError("");
	document.getElementById('output').style.display='none';
	document.getElementById("data").value="";
}

document.getElementById('plot').onclick = function() {

	writeError("");
	document.getElementById('output').style.display='none';

	var data = document.getElementById("data").value;
	if (data == null || data == "") {
		writeError("Please paste some data into the text area above and press 'Plot' again");
		return;
	}

	var breakup = data.split("\n");
	if (breakup.length != 54 && breakup.length != 57 && breakup.length != 56)  {
		writeError("There must be 54, 56 or 57 lines pasted into the text area");
		return;
	}

	var base = new Date().valueOf();
	var graph = new Array();

	var fromhr = "06";
	var frommin = "15";
	var tohr = "08";
	var tomin = "00";
	var smart = "N";
	var goneoff = "N";

	for (var i=0; i < breakup.length; i++) {
		var lineSplit = breakup[i].split(",");
		if (lineSplit.length != 2) {
			writeError("Each line must be a comma separated pair of values");
			return;
		}
		var timeComp = lineSplit[0].trim();
		var valueComp = lineSplit[1].trim().toUpperCase();
		if (i==0) {
			var baseStr = new Date().format('yyyy/MM/dd') + ' ' + timeComp + ":00";
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

	document.getElementById('output').style.display='block';

	var smartOn = smart == 'Y';

//	Build datasets for the graph
	var startPoint = new Date(base);

	var splitup = graph;
	var more = new Array();

	for (var i = 0; i < splitup.length; i++) {
		if (splitup[i] == '')
			continue;
		var element = new Array();
		element[0] = startPoint;
		element[1] = parseInt(splitup[i], 10);
		if (element[1] == -1)
			element[1] = null;
		more[i] = element;
		startPoint = startPoint.addMinutes(mConst().sampleIntervalMins);
	}

//	Work out where the start and stop of the wake-up period should go
	var canvasOverlayConf;
	if (smartOn) {
		var fromstr = fixLen(fromhr) + fixLen(frommin);
		var tostr = fixLen(tohr) + fixLen(tomin);
		var smartStartPoint = new Date(base);
		var early = null;
		var late = null;
		var actual = null;
		for (var i = 0; i < splitup.length; i++) {
			var teststr1 = smartStartPoint.format("hhmm");
			var smartStartPoint1 = smartStartPoint;
			smartStartPoint = smartStartPoint
			.addMinutes(mConst().sampleIntervalMins);
			var teststr2 = smartStartPoint.format("hhmm");
			if (early == null && fromstr >= teststr1 && fromstr <= teststr2)
				early = smartStartPoint1;
			if (late == null && tostr >= teststr1 && tostr <= teststr2)
				late = smartStartPoint1;
			if (actual == null && goneoff != 'N' && goneoff >= teststr1
					&& goneoff <= teststr2)
				actual = smartStartPoint1;
			if (late != null && early != null && actual != null)
				break;
		}
		if (early != null && late != null) {
			var canvasOverlayActual;
			if (actual != null) {
				canvasOverlayActual = {
						verticalLine : {
							name : "actual",
							x : actual,
							lineWidth : 1,
							yOffset : 0,
							color : 'rgb(255, 149, 0)',
							shadow : false
						}
				};
			}
			canvasOverlayConf = {
					show : true,
					objects : [ {
						dashedVerticalLine : {
							name : "start",
							x : early,
							lineWidth : 1,
							yOffset : 0,
							dashPattern : [ 1, 4 ],
							color : 'rgb(76, 217, 100)',
							shadow : false
						}
					}, {
						dashedVerticalLine : {
							name : "end",
							x : late,
							lineWidth : 1,
							yOffset : 0,
							dashPattern : [ 1, 4 ],
							color : 'rgb(255, 59, 48)',
							shadow : false
						}
					}, canvasOverlayActual ]
			};
		}
	}

//	Get the full set of data up to the wake up point.
//	Ignore nulls
	var pieStartPoint = new Date(base);
	var points = 0;
	var total = 0;
	var awake = 0;
	var deep = 0;
	var light = 0;
	for (var i = 0; i < splitup.length; i++) {
		if (splitup[i] == '')
			continue;
		var data = parseInt(splitup[i], 10);
		var teststr1 = pieStartPoint.format("hhmm");
		var pieStartPoint1 = pieStartPoint;
		pieStartPoint = pieStartPoint.addMinutes(mConst().sampleIntervalMins);
		var teststr2 = pieStartPoint.format("hhmm");
		if (goneoff != 'N' && goneoff >= teststr1 && goneoff <= teststr2)
			break;
		if (data == -1)
			continue;
		if (data > mConst().awakeAbove) {
			awake++;
		} else if (data > mConst().lightAbove) {
			light++;
		} else {
			deep++;
		}
	}

	document.getElementById('ttotal').textContent = hrsmin((deep + light + awake)
			* mConst().sampleIntervalMins);
	document.getElementById('tawake').textContent = hrsmin(awake
			* mConst().sampleIntervalMins);
	document.getElementById('tlight').textContent = hrsmin(light
			* mConst().sampleIntervalMins);
	document.getElementById('tdeep').textContent = hrsmin(deep
			* mConst().sampleIntervalMins);

	var data2 = [ [ 'Awake?', awake ], [ 'Light', light ], [ 'Deep', deep ] ];

//	Prepare the graph
	var plot1 = $.jqplot('chart1', [ more ], {
		grid : {
			background : '#ffffff'
		},
		animate : true,
		canvasOverlay : canvasOverlayConf,
		series : [ {
			showMarker : false,
			breakOnNull : true,
			color : 'rgb(0,122,255)',
			label : new Date(base).format('yyyy-MM-dd'),
			trendline : {
				show : true,
				color : 'rgb(90,200,250)',
				label : '',
				type : 'linear',
				shadow : true,
				lineWidth : 1.5,
				shadowAngle : 45,
				shadowOffset : 1.5,
				shadowDepth : 3,
				shadowAlpha : 0.07
			}
		} ],
		// You can specify options for all axes on the plot at once with
		// the axesDefaults object. Here, we're using a canvas renderer
		// to draw the axis label which allows rotated text.
		axesDefaults : {
			labelRenderer : $.jqplot.CanvasAxisLabelRenderer,
			showTicks : false,
			showTickMarks : false,
		},
		legend : {
			show : false,
			location : 'ne'
		},
		// An axes object holds options for all axes.
		// Allowable axes are xaxis, x2axis, yaxis, y2axis, y3axis, ...
		// Up to 9 y axes are supported.
		axes : {
			// options for each axis are specified in seperate option
			// objects.
			xaxis : {
				renderer : $.jqplot.DateAxisRenderer,
				tickRenderer : $.jqplot.CanvasAxisTickRenderer,
				tickOptions : {
					formatString : '%R',
					angle : -30,
					fontSize : '8pt'
				},
				tickInterval : '1 hour',
				// Turn off "padding". This will allow data point to lie
				// on the
				// edges of the grid. Default padding is 1.2 and will
				// keep all
				// points inside the bounds of the grid.
				pad : 0,
				showTicks : true,
				showTickMarks : true,
				min : new Date(base)
			},
			yaxis : {
				label : "Movement",
				min : -50,
				max : 4000,
			}
		}
	});

	var plot2 = jQuery.jqplot('chart2', [ data2 ], {
		grid : {
			background : '#ffffff'
		},
		seriesColors : [ "rgb(90, 200, 250)", "rgb(0,122,255)",
		                 "rgb(88,86,214)" ],
		                 seriesDefaults : {
		                	 // Make this a pie chart.
		                	 renderer : jQuery.jqplot.PieRenderer,
		                	 rendererOptions : {
		                		 // Put data labels on the pie slices.
		                		 // By default, labels show the percentage of the slice.
		                		 showDataLabels : true
		                	 }
		                 },
		                 legend : {
		                	 show : true,
		                	 location : 'e'
		                 }
	});

}


