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
function mConst() {
  return {
    awakeAbove : 1000,
    lightAbove : 120,
    sampleIntervalMins : 10,
    swpAppStoreUrl : "https://itunes.apple.com/app/smartwatch-pro-for-pebble/id673907094?mt=8&at=10lIFm&ct=morpheuz_web",
    displayDateFmt : "WWW, NNN dd, yyyy hh:mm",
    iosDateFormat : "dd N yyyy hh:mm",
    swpUrlDate : "yyyy-MM-ddThh:mm:00"
  };
}

/*
 * Build data set for the graph
 */
function buildGraphDataSet(base, splitup, more) {
  var startPoint = new Date(base);
  for (var i = 0; i < splitup.length; i++) {
    if (splitup[i] === "") {
      continue;
    }
    var element = new Array();
    element[0] = startPoint;
    element[1] = parseInt(splitup[i], 10);
    if (element[1] < 0) {
      element[1] = null;
    }
    more[i] = element;
    startPoint = startPoint.addMinutes(mConst().sampleIntervalMins);
  }
}

/*
 * Populate ignore segments
 */
function populateIgnore(base, canvasOverlayConf, splitup) {

  var startPoint = new Date(base);
  for (var i = 0; i < splitup.length; i++) {
    if (splitup[i] === "") {
      continue;
    }
    if (parseInt(splitup[i], 10) == -2) {
      var ignoreOverlay = {
        verticalLine : {
          name : "ignore",
          x : startPoint,
          lineWidth : 5,
          yOffset : 0,
          color : "#184E99",
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
 * Locate match one minute at a time
 */
function returnAbsoluteMatch(early, late, actualstr) {
  var point = early;
  while (point.getTime() < late.getTime()) {
    var teststr = point.format("hhmm");
    if (actualstr === teststr) {
      return point;
    }
    point = point.addMinutes(1);
  }
  return early;
}

/*
 * Work out where the start and stop of the wake-up period should go
 */
function startStopAlarm(smartOn, fromhr, frommin, tohr, tomin, base, canvasOverlayConf, splitup) {
  if (smartOn) {
    var fromstr = fixLen(fromhr) + fixLen(frommin);
    var tostr = fixLen(tohr) + fixLen(tomin);
    var smartStartPoint = new Date(base);
    var early = null;
    var late = null;
    for (var i = 0; i < splitup.length; i++) {
      var teststr1 = smartStartPoint.format("hhmm");
      var smartStartPoint1 = smartStartPoint;
      smartStartPoint = smartStartPoint.addMinutes(mConst().sampleIntervalMins);
      var teststr2 = smartStartPoint.format("hhmm");
      if (early === null && fromstr >= teststr1 && fromstr <= teststr2) {
        early = returnAbsoluteMatch(smartStartPoint1, smartStartPoint, fromstr);
      }
      if (late === null && tostr >= teststr1 && tostr <= teststr2) {
        late = returnAbsoluteMatch(smartStartPoint1, smartStartPoint, tostr);
      }
      if (late !== null && early !== null) {
        break;
      }
    }
    if (early !== null) {
      var earlyOverlay = {
        dashedVerticalLine : {
          name : "start",
          x : early,
          lineWidth : 1,
          yOffset : 0,
          dashPattern : [ 1, 4 ],
          color : "rgb(76, 217, 100)",
          shadow : false
        }
      };
      canvasOverlayConf.show = true;
      canvasOverlayConf.objects.push(earlyOverlay);
    }
    if (late !== null) {
      var lateOverlay = {
        dashedVerticalLine : {
          name : "end",
          x : late,
          lineWidth : 1,
          yOffset : 0,
          dashPattern : [ 1, 4 ],
          color : "rgb(255, 59, 48)",
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
  var firstSleep = true;
  var tbegin = null;
  var ibegin = null;
  var tends = null;
  var iends = null;
  var tendsStop = null;
  var iendsStop = null;
  for (var i = 0; i < splitup.length; i++) {
    if (splitup[i] === "") {
      continue;
    }
    var data = parseInt(splitup[i], 10);
    var teststr1 = pieStartPoint.format("hhmm");
    var pieStartPoint1 = pieStartPoint;
    pieStartPoint = pieStartPoint.addMinutes(mConst().sampleIntervalMins);
    var teststr2 = pieStartPoint.format("hhmm");
    if (goneoff != "N" && goneoff >= teststr1 && goneoff <= teststr2) {
      tends = returnAbsoluteMatch(pieStartPoint1, pieStartPoint, goneoff);
      iends = i;
      break;
    } else if (data != -1 && data != -2 && data <= mConst().awakeAbove) {
      if (firstSleep) {
        tbegin = pieStartPoint1;
        ibegin = i;
        var beginOverlay = {
          verticalLine : {
            name : "begin",
            x : tbegin,
            lineWidth : 1,
            yOffset : 0,
            color : "rgb(255, 149, 0)",
            shadow : false
          }
        };
        canvasOverlayConf.show = true;
        canvasOverlayConf.objects.push(beginOverlay);
        firstSleep = false;
      }
      tendsStop = pieStartPoint;
      iendsStop = i;
    }
  }

  // If we haven't got a regular end because of an alarm, then find
  // the last time they were below waking levels of movement
  if (tends === null && tendsStop !== null) {
    tends = tendsStop;
    iends = iendsStop;
  }

  // Compute the stats within the bounds of the start and stop times
  var awake = 0;
  var deep = 0;
  var light = 0;
  var ignore = 0;
  if (ibegin !== null && iends !== null) {
    for (var j = ibegin; j <= iends; j++) {
      if (splitup[j] === "") {
        continue;
      }
      var data2 = parseInt(splitup[j], 10);
      if (data2 == -1 || data2 == -2) {
        ignore++;
      } else if (data2 > mConst().awakeAbove) {
        awake++;
      } else if (data2 > mConst().lightAbove) {
        light++;
      } else {
        deep++;
      }
    }
  }

  // Add the end time overlay
  if (tends !== null) {
    var endsStopOverlay = {
      verticalLine : {
        name : "endstop",
        x : tends,
        lineWidth : 1,
        yOffset : 0,
        color : "rgb(255, 149, 0)",
        shadow : false
      }
    };
    canvasOverlayConf.show = true;
    canvasOverlayConf.objects.push(endsStopOverlay);
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
function generateCopyLinkData(base, splitup, smartOn, fromhr, frommin, tohr, tomin, goneoff) {

  var timePoint = new Date(base);
  var body = "&body=";
  var copyBody = "";

  for (var i = 0; i < splitup.length; i++) {
    if (splitup[i] === "") {
      continue;
    }
    body = body + timePoint.format("hh:mm") + "," + splitup[i] + "%0D%0A";
    copyBody = copyBody + timePoint.format("hh:mm") + "," + splitup[i] + "\r\n";
    timePoint = timePoint.addMinutes(mConst().sampleIntervalMins);
  }

  // Add smart alarm info into CSV data
  if (smartOn) {
    body = body + fromhr + ":" + frommin + ",START%0D%0A" + tohr + ":" + tomin + ",END%0D%0A";
    copyBody = copyBody + fromhr + ":" + frommin + ",START\r\n" + tohr + ":" + tomin + ",END\r\n";
    if (goneoff != "N") {
      var goneoffstr = goneoff.substr(0, 2) + ":" + goneoff.substr(2, 2);
      body = body + goneoffstr + ",ALARM%0D%0A";
      copyBody = copyBody + goneoffstr + ",ALARM\r\n";
    }
  }
  return {
    "body" : body,
    "copyBody" : copyBody
  };
}

/*******************************************************************************
 * 
 * Main process
 * 
 ******************************************************************************/

$("document").ready(function() {

  // Adjust page for viewport
  adjustForViewport();

  // Spot if we are on iOS or not
  document.ios = navigator.userAgent.match(/iPhone/i) || navigator.userAgent.match(/iPad/i) || navigator.userAgent.match(/iPod/i);

  if (document.ios) {
    $(".android").hide();
  } else {
    $(".ios").hide();
  }

  // Pick up parameters from URL
  var baseStr = getParameterByName("base");
  var base = new Date().valueOf();
  if (baseStr !== "" && baseStr !== "null") {
    base = parseInt(baseStr, 10);
  }
  var graph = getParameterByName("graph");
  var fromhr = getParameterByName("fromhr");
  var frommin = getParameterByName("frommin");
  var tohr = getParameterByName("tohr");
  var tomin = getParameterByName("tomin");
  var smart = getParameterByName("smart");
  var vers = getParameterByName("vers");
  var goneoff = getParameterByName("goneoff");
  var emailto = decodeURIComponent(getParameterByName("emailto"));
  var potoken = decodeURIComponent(getParameterByName("potoken"));
  var pouser = decodeURIComponent(getParameterByName("pouser"));
  var postat = decodeURIComponent(getParameterByName("postat"));
  var swpdo = decodeURIComponent(getParameterByName("swpdo"));
  var swpstat = decodeURIComponent(getParameterByName("swpstat"));
  var noset = getParameterByName("noset");
  var token = getParameterByName("token");
  var exptime = decodeURIComponent(getParameterByName("exptime"));
  var usage = getParameterByName("usage");

  var smartOn = smart === "Y";
  var nosetOn = noset === "Y";
  if (nosetOn) {
    $(".noset").hide();
  }

  // Set screen fields
  $("#emailto").val(emailto);
  $("#ptoken").val(potoken);
  $("#puser").val(pouser);
  $("#swpdo").prop("checked", swpdo === "Y");
  $("#presult").text(postat);
  $("#swpstat").text(swpstat);
  $("#exptime").text(exptime);
  $("#usage").prop("checked", usage !== "N");

  // Set the status bullets for pushover
  if (postat === "OK") {
    $("#lipushover").addClass("green");
    $("#presult").addClass("green");
  } else if (postat === "" || postat === null || postat === "Disabled") {
    $("#lipushover").addClass("blue");
    $("#presult").addClass("blue");
  } else {
    $("#lipushover").addClass("red");
    $("#presult").addClass("red");
  }

  // Set the status bullets for smartwach pro
  if (swpstat === "OK") {
    $("#liswp").addClass("green");
    $("#swpstat").addClass("green");
  } else if (swpstat === "" || swpstat === null || swpstat === "Disabled") {
    $("#liswp").addClass("blue");
    $("#swpstat").addClass("blue");
  } else {
    $("#liswp").addClass("red");
    $("#swpstat").addClass("red");
  }

  $("#version").text(parseInt(vers, 10) / 10);
  $("#sleep-time").text(new Date(base).format(mConst().displayDateFmt));

  if ((new Date().valueOf()) % 10 === 0) {
    $("#info-message").css("display", "block");
  }

  // Show version warning
  if (!nosetOn) {
    setScreenMessageBasedOnVersion(vers);
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
  startStopAlarm(smartOn, fromhr, frommin, tohr, tomin, base, canvasOverlayConf, splitup);

  // Return stats
  var out = calculateStats(base, splitup, goneoff, canvasOverlayConf);

  // Populate the statistics area
  $("#ttotal").text(hrsmin((out.deep + out.light + out.awake + out.ignore) * mConst().sampleIntervalMins));
  $("#tawake").text(hrsmin(out.awake * mConst().sampleIntervalMins));
  $("#tlight").text(hrsmin(out.light * mConst().sampleIntervalMins));
  $("#tdeep").text(hrsmin(out.deep * mConst().sampleIntervalMins));
  $("#tignore").text(hrsmin(out.ignore * mConst().sampleIntervalMins));

  // If we have a begin and an end then show this in our 'HealthKit' datapoint
  // section and
  // Make it exportable
  if (out.tbegin !== null && out.tends !== null) {
    $("#swpnodata").hide();
    $("#tstarts").text(out.tbegin.format(mConst().iosDateFormat));
    $("#tends").text(out.tends.format(mConst().iosDateFormat));
    var swpUrl = "swpro2hk://?source=Morpheuz&starts=" + out.tbegin.format(mConst().swpUrlDate) + "&ends=" + out.tends.format(mConst().swpUrlDate);
    if (token != null && token !== "") {
      swpUrl += "&at=" + token;
    }
    $("#swp").prop("href", swpUrl);
    if (!nosetOn) {
      $("#swp").click(function() {
        setTimeout(function() {
          window.location.href = "pebblejs://close";
        }, 250);
      });
    }
  } else {
    $("#swp").hide();
  }

  $("#swplink").prop("href", mConst().swpAppStoreUrl);
  $("#swplink").click(function() {
    setTimeout(function() {
      window.location.href = "pebblejs://close";
    }, 250);
  });

  // Build the pie chart data
  var data2 = [ [ "Awake?", out.awake ], [ "Light", out.light ], [ "Deep", out.deep ], [ "Ignore", out.ignore ] ];

  // Prepare the graph
  $(document).ready(function() {

    document.plot1 = $.jqplot("chart1", [ more ], {
      grid : {
        background : "#2066C7",
        gridLineColor : "#1E75D7",
        borderColor : "#1E75D7",
        shadow : false
      },
      animate : true,
      canvasOverlay : canvasOverlayConf,
      series : [ {
        showMarker : false,
        breakOnNull : true,
        color : "#40ADEB",
        label : new Date(base).format("yyyy-MM-dd"),
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
        location : "ne"
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
            formatString : "%R",
            angle : -30,
            fontSize : "8pt",
            textColor : "#40ADEB"
          },
          tickInterval : "1 hour",
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
            textColor : "#1898FF"
          }
        }
      }
    });

    document.plot2 = jQuery.jqplot("chart2", [ data2 ], {
      grid : {
        background : "#FF7D48",
        borderColor : "#FF7D48",
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
        location : "e"
      }
    });

  });

  // Generate data to copy and email
  var cpy = generateCopyLinkData(base, splitup, smartOn, fromhr, frommin, tohr, tomin, goneoff);

  var mailto = "?subject=Morpheuz-" + new Date(base).format("yyyy-MM-dd") + ".csv" + cpy.body;

  $("#mailtemp").val(mailto);
  $("#mail").prop("href", "mailto:" + emailto + mailto);
  $("#copy").val(cpy.copyBody);

  if (!nosetOn) {
    $("#mail").click(function() {
      setTimeout(function() {
        window.location.href = "pebblejs://close";
      }, 250);
    });
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
  $("#emailto").change(function() {
    var mailtemp = $("#mailtemp").val();
    var emailto = $("#emailto").val();
    $("#mail").prop("href", "mailto:" + emailto + mailtemp);
  });

  // Handle the Save and reset option
  $(".save").click(function() {
    var unused = "N";
    var blank = "";
    var emailpart = encodeURIComponent($("#emailto").val());
    var potoken = encodeURIComponent($("#ptoken").val());
    var pouser = encodeURIComponent($("#puser").val());
    var swpdo = $("#swpdo").is(':checked') ? "Y" : "N";
    var usage = $("#usage").is(':checked') ? "Y" : "N";
    window.location.href = "pebblejs://close#reset" + "!" + unused + "!" + blank + "!" + blank + "!" + blank + "!" + blank + "!" + unused + "!" + emailpart + "!" + pouser + "!" + blank + "!" + potoken + "!" + unused + "!" + swpdo + "!" + usage;
  });

});
