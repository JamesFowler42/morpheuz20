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
		awakeAbove : 1000,
		lightAbove : 120,
		sampleIntervalMins : 10,
		vers : 23
	};
	return cfg;
}

/*
 * Build data set for the graph
 */
function buildGraphDataSet(base, splitup, more) {
	var startPoint = new Date(base);
	for (var i = 0; i < splitup.length; i++) {
		if (splitup[i] == '')
			continue;
		var element = new Array();
		element[0] = startPoint;
		element[1] = parseInt(splitup[i], 10);
		if (element[1] < 0)
			element[1] = null;
		more[i] = element;
		startPoint = startPoint.addMinutes(mConst().sampleIntervalMins);
	}
}

/*
 * Populate ignore segments
 */
function populateIgnore(base, canvasOverlayConf, splitup) {

	// Populate with ignore segments
	var startPoint = new Date(base);
	for (var i = 0; i < splitup.length; i++) {
		if (splitup[i] == '')
			continue;
		if (parseInt(splitup[i], 10) == -2) {
			var ignoreOverlay = {
				verticalLine : {
					name : "ignore",
					x : startPoint,
					lineWidth : 5,
					yOffset : 0,
					color : '#184E99',
					shadow : false
				}
			};
			canvasOverlayConf.show = true;
			canvasOverlayConf.objects.push(ignoreOverlay);
		}
		startPoint = startPoint.addMinutes(mConst().sampleIntervalMins);
	}
}

/*
 * Work out where the start and stop of the wake-up period should go
 */
function startStopAlarm(smartOn, fromhr, frommin, tohr, tomin, base,
		canvasOverlayConf, goneoff, splitup) {
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
			if (actual != null) {
				var canvasOverlayActual = {
					verticalLine : {
						name : "actual",
						x : actual,
						lineWidth : 1,
						yOffset : 0,
						color : 'rgb(255, 149, 0)',
						shadow : false
					}
				};
				canvasOverlayConf.show = true;
				canvasOverlayConf.objects.push(canvasOverlayActual);
			}
			var earlyOverlay = {
				dashedVerticalLine : {
					name : "start",
					x : early,
					lineWidth : 1,
					yOffset : 0,
					dashPattern : [ 1, 4 ],
					color : 'rgb(76, 217, 100)',
					shadow : false
				}
			};
			canvasOverlayConf.show = true;
			canvasOverlayConf.objects.push(earlyOverlay);
			var lateOverlay = {
				dashedVerticalLine : {
					name : "end",
					x : late,
					lineWidth : 1,
					yOffset : 0,
					dashPattern : [ 1, 4 ],
					color : 'rgb(255, 59, 48)',
					shadow : false
				}
			};
			canvasOverlayConf.show = true;
			canvasOverlayConf.objects.push(lateOverlay);
		}
	}
}

/*
 * Calculate stats
 */
function calculateStats(base, splitup, goneoff, canvasOverlayConf) {
	// Get the full set of data up to the wake up point.
	// Ignore nulls
	var pieStartPoint = new Date(base);
	var points = 0;
	var total = 0;
	var awake = 0;
	var deep = 0;
	var light = 0;
	var ignore = 0;
	var firstSleep = true;
	var tbegin = null;
	var tends = null;
	var tendsStop = null;
	for (var i = 0; i < splitup.length; i++) {
		if (splitup[i] == '')
			continue;
		var data = parseInt(splitup[i], 10);
		var teststr1 = pieStartPoint.format("hhmm");
		var pieStartPoint1 = pieStartPoint;
		pieStartPoint = pieStartPoint.addMinutes(mConst().sampleIntervalMins);
		var teststr2 = pieStartPoint.format("hhmm");
		if (goneoff != 'N' && goneoff >= teststr1 && goneoff <= teststr2) {
			tends = pieStartPoint;
			break;
		} else if (data == -1) {
			continue;
		} else if (data == -2) {
			ignore++;
		} else if (data > mConst().awakeAbove) {
			awake++;
		} else {
			if (firstSleep) {
				tbegin = pieStartPoint;
				var beginOverlay = {
					verticalLine : {
						name : "begin",
						x : tbegin,
						lineWidth : 1,
						yOffset : 0,
						color : 'rgb(255, 149, 0)',
						shadow : false
					}
				};
				canvasOverlayConf.show = true;
				canvasOverlayConf.objects.push(beginOverlay);
				firstSleep = false;
			}
			tendsStop = pieStartPoint;
			if (data > mConst().lightAbove) {
				light++;
			} else {
				deep++;
			}
		}
	}

	// If we haven't got a regular end because of an alarm, then find
	// the last time they were below waking levels of movement
	if (tends == null && tendsStop != null) {
		var endsStopOverlay = {
			verticalLine : {
				name : "endstop",
				x : tendsStop,
				lineWidth : 1,
				yOffset : 0,
				color : 'rgb(255, 149, 0)',
				shadow : false
			}
		};
		canvasOverlayConf.show = true;
		canvasOverlayConf.objects.push(endsStopOverlay);
		tends = tendsStop;
	}
	return {
		"tbegin" : tbegin,
		"tends" : tends,
		"deep" : deep,
		"light" : light,
		"awake" : awake,
		"ignore" : ignore
	};
}

/*
 * Prepare the data for the copy links
 */
function generateCopyLinkData(base, splitup, smartOn, fromhr, frommin, tohr,
		tomin, goneoff) {

	var timePoint = new Date(base);
	var body = '&body=';
	var copyBody = '';

	for (var i = 0; i < splitup.length; i++) {
		if (splitup[i] == '')
			continue;
		body = body + timePoint.format('hh:mm') + ',' + splitup[i] + '%0D%0A';
		copyBody = copyBody + timePoint.format('hh:mm') + ',' + splitup[i]
				+ "\r\n";
		timePoint = timePoint.addMinutes(mConst().sampleIntervalMins);
	}

	// Add smart alarm info into CSV data
	if (smartOn) {
		body = body + fromhr + ':' + frommin + ',START%0D%0A' + tohr + ':'
				+ tomin + ',END%0D%0A';
		copyBody = copyBody + fromhr + ':' + frommin + ',START\r\n' + tohr
				+ ':' + tomin + ',END\r\n';
		if (goneoff != 'N') {
			var goneoffstr = goneoff.substr(0, 2) + ':' + goneoff.substr(2, 2);
			body = body + goneoffstr + ',ALARM%0D%0A';
			copyBody = copyBody + goneoffstr + ',ALARM\r\n';
		}
	}
	return {
		"body" : body,
		"copyBody" : copyBody
	};
}

/*
 * Get element
 */
function getEl(name) {
	return document.getElementById(name);
}

/*******************************************************************************
 * 
 * Main process
 * 
 ******************************************************************************/

// Spot if we are on iOS or not
document.ios = navigator.userAgent.match(/iPhone/i)
		|| navigator.userAgent.match(/iPad/i)
		|| navigator.userAgent.match(/iPod/i);

if (document.ios) {
	$(".android").hide();
} else {
	$(".ios").hide();
}

// Pick up parameters from URL
var baseStr = getParameterByName('base');
var base = new Date().valueOf();
if (baseStr != "" && baseStr != 'null') {
	base = parseInt(baseStr, 10);
}
var graph = getParameterByName("graph");
var fromhr = getParameterByName("fromhr");
var frommin = getParameterByName("frommin");
var tohr = getParameterByName("tohr");
var tomin = getParameterByName("tomin");
var smart = getParameterByName("smart");
var inverse = getParameterByName("inverse");
var vers = getParameterByName("vers");
var goneoff = getParameterByName("goneoff");
var emailto = decodeURIComponent(getParameterByName('emailto'));
var xuser = decodeURIComponent(getParameterByName('xuser'));
var xpass = decodeURIComponent(getParameterByName('xpass'));
var xkey = decodeURIComponent(getParameterByName('xkey'));

var smartOn = smart == 'Y';
var inverseOn = inverse == 'Y';

// Set screen fields
getEl('emailto').value = emailto;
getEl('smartalarm').checked = smartOn;
getEl('inverse').checked = inverseOn;

if (document.ios) {
	setSelectedValue(getEl('safromhour'), fromhr);
	setSelectedValue(getEl('safrommin'), frommin);
	setSelectedValue(getEl('satohour'), tohr);
	setSelectedValue(getEl('satomin'), tomin);
} else {
	setTextValue(getEl('safromhourtxt'), fromhr);
	setTextValue(getEl('safrommintxt'), frommin);
	setTextValue(getEl('satohourtxt'), tohr);
	setTextValue(getEl('satomintxt'), tomin);
}

getEl("version").textContent = parseInt(vers, 10) / 10;

if (vers != mConst().vers) {
	getEl('verserror').style.display = 'inline';
}

var splitup = graph.split("!");
var more = new Array();

// Build graph data
buildGraphDataSet(base, splitup, more);

// Declare canvas overlay which will be populated as we go on
var canvasOverlayConf = {
	show : false,
	objects : []
};

// Build ignore bars
populateIgnore(base, canvasOverlayConf, splitup);

// Return start and stop times
startStopAlarm(smartOn, fromhr, frommin, tohr, tomin, base, canvasOverlayConf,
		goneoff, splitup);

// Return stats
var out = calculateStats(base, splitup, goneoff, canvasOverlayConf);

// Populate the statistics area
getEl('ttotal').textContent = hrsmin((out.deep + out.light + out.awake + out.ignore)
		* mConst().sampleIntervalMins);
getEl('tawake').textContent = hrsmin(out.awake * mConst().sampleIntervalMins);
getEl('tlight').textContent = hrsmin(out.light * mConst().sampleIntervalMins);
getEl('tdeep').textContent = hrsmin(out.deep * mConst().sampleIntervalMins);
getEl('tignore').textContent = hrsmin(out.ignore * mConst().sampleIntervalMins);

// If we have a begin and an end then show this in our 'HealthKit' datapoint
// section and
// Make it exportable
if (out.tbegin != null && out.tends != null) {
	getEl('tstarts').textContent = out.tbegin.format('dd N yyyy hh:mm');
	getEl('tends').textContent = out.tends.format('dd N yyyy hh:mm');
	getEl('swp').href = "swpro2hk://?source=Morpheuz&starts="
			+ out.tbegin.format('yyyy-MM-ddThh:mm:00') + "&ends="
			+ out.tends.format('yyyy-MM-ddThh:mm:00');
	getEl('swp').onclick = function() {
		setTimeout(function() {
			window.location.href = 'pebblejs://close';
		}, 250);
	}
} else {
	$("#swp").hide();
}

// Build the pie chart data
var data2 = [ [ 'Awake?', out.awake ], [ 'Light', out.light ],
		[ 'Deep', out.deep ], [ 'Ignore', out.ignore ] ];

// Prepare the graph
$(document).ready(function() {

	var plot1 = $.jqplot('chart1', [ more ], {
		grid : {
			background : '#2066C7',
			gridLineColor : '#1E75D7',
			borderColor : '#1E75D7',
			shadow : false
		},
		animate : true,
		canvasOverlay : canvasOverlayConf,
		series : [ {
			showMarker : false,
			breakOnNull : true,
			color : '#40ADEB',
			label : new Date(base).format('yyyy-MM-dd'),
			shadow : false
		} ],
		// You can specify options for all axes on the plot at once with
		// the axesDefaults object. Here, we're using a canvas renderer
		// to draw the axis label which allows rotated text.
		axesDefaults : {
			labelRenderer : $.jqplot.CanvasAxisLabelRenderer,
			showTicks : false,
			showTickMarks : false
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
					fontSize : '8pt',
					textColor : '#1898FF'
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
				labelRenderer : $.jqplot.CanvasAxisLabelRenderer,
				label : "Movement",
				min : -50,
				max : 4000,
				labelOptions : {
					textColor : '#1898FF'
				}
			}
		}
	});

	var plot2 = jQuery.jqplot('chart2', [ data2 ], {
		grid : {
			background : '#FF7D48',
			borderColor : '#FF7D48',
			shadow : false
		},
		seriesColors : [ "#FFFF92", "#FFA966", "#FF3C31", "rgb(130,130,130)" ],
		seriesDefaults : {
			// Make this a pie chart.
			renderer : jQuery.jqplot.PieRenderer,
			rendererOptions : {
				// Put data labels on the pie slices.
				// By default, labels show the percentage of the slice.
				showDataLabels : true,
				shadow : false
			}
		},
		legend : {
			show : true,
			location : 'e'
		}
	});

});

// Generate data to copy and email
var cpy = generateCopyLinkData(base, splitup, smartOn, fromhr, frommin, tohr,
		tomin, goneoff);

var mailto = '?subject=Morpheuz-' + new Date(base).format('yyyy-MM-dd')
		+ '.csv' + cpy.body;

getEl('mailtemp').value = mailto;
getEl('mail').href = 'mailto:' + emailto + mailto;
getEl('copy').value = cpy.copyBody;

getEl('mail').onclick = function() {
	setTimeout(function() {
		window.location.href = 'pebblejs://close';
	}, 250);
}

$("#copy").focus(function() {
	var $this = $(this);
	$this.select();

	// Work around Chrome's little problem
	$this.mouseup(function() {
		// Prevent further mouseup intervention
		$this.unbind("mouseup");
		return false;
	});
});

// Change the mailto
getEl('emailto').onchange = function() {
	var mailtemp = getEl('mailtemp').value;
	var emailto = getEl('emailto').value;
	getEl('mail').href = 'mailto:' + emailto + mailtemp;
}

// Handle the Save and reset option
getEl('reset').onclick = function() {
	var smartpart = getEl('smartalarm').checked ? 'Y' : 'N';
	var inversepart = getEl('inverse').checked ? 'Y' : 'N';
	var emailpart = encodeURIComponent(getEl('emailto').value);
	var xuserpart = "";
	var xpasspart = "";
	var xkeypart = "";
	var fromhrpart = "";
	var fromminpart = "";
	var tohrpart = "";
	var tominpart = "";
	if (document.ios) {
		fromhrpart = getSelectedValue(getEl('safromhour'));
		fromminpart = getSelectedValue(getEl('safrommin'));
		tohrpart = getSelectedValue(getEl('satohour'));
		tominpart = getSelectedValue(getEl('satomin'));
	} else {
		fromhrpart = getTextValue(getEl('safromhourtxt'), true);
		fromminpart = getTextValue(getEl('safrommintxt'), false);
		tohrpart = getTextValue(getEl('satohourtxt'), true);
		tominpart = getTextValue(getEl('satomintxt'), false);
		if (fromhrpart == "X" || fromminpart == "X" || tohrpart == "X"
				|| tominpart == "X")
			return;
	}

	function setRed(item) {
		getEl(item).style.backgroundColor = 'red';
	}

	if ((fromhrpart + ":" + fromminpart) >= (tohrpart + ":" + tominpart)) {
		setRed('safromhour');
		setRed('safrommin');
		setRed('satohour');
		setRed('satomin');
		setRed('safromhourtxt');
		setRed('safrommintxt');
		setRed('satohourtxt');
		setRed('satomintxt');
	} else {
		window.location.href = 'pebblejs://close#reset' + '!' + smartpart + '!'
				+ fromhrpart + '!' + fromminpart + '!' + tohrpart + '!'
				+ tominpart + '!' + inversepart + '!' + emailpart + '!'
				+ xuserpart + '!' + xpasspart + '!' + xkeypart;
	}
}
