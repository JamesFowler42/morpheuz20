/* 
 * Morpheuz Sleep Monitor
 *
 * Copyright (c) 2013-2016 James Fowler
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
    chartBottom : -50,
    chartTop : 4000,
    swpAppStoreUrl : "https://itunes.apple.com/app/smartwatch-pro-for-pebble/id673907094?mt=8&at=10lIFm&pt=409665&ct=morpheuz_web",
    displayDateFmt : "WWW, NNN dd, yyyy hh:mm",
    iosDateFormat : "dd N yyyy hh:mm",
    swpUrlDate : "yyyy-MM-ddThh:mm:00",
    emailAddressMandatory : "valid email address is required",
    sendingEmail : "Sending...",
    url : "http://ui.morpheuz.net/morpheuz/view-",
    twitterWebIntentUrl : "https://twitter.com/intent/tweet?hashtags=morpheuz,tweetMySleep&text=",
    unableToFindTweetText : "Meh",
    numberOfSamples : 60
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
    startPoint = startPoint.addMinutes(MorpheuzCommon.mCommonConst().sampleIntervalMins);
  }
}

/*
 * Populate ignore segments
 */
function populateIgnore(base, canvasOverlayConf, splitup, totalWidth) {

  var lineW = totalWidth / splitup.length;

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
          lineWidth : lineW,
          yOffset : 0,
          color : "#AAAAAA",
          shadow : false
        }
      };
      canvasOverlayConf.show = true;
      canvasOverlayConf.objects.push(ignoreOverlay);
    }
    startPoint = startPoint.addMinutes(MorpheuzCommon.mCommonConst().sampleIntervalMins);
  }
}

/*
 * Work out where the start and stop of the wake-up period should go
 */
function startStopAlarm(smartOn, fromhr, frommin, tohr, tomin, base, canvasOverlayConf, splitup) {
  if (smartOn) {
    var fromstr = MorpheuzCommon.fixLen(fromhr) + MorpheuzCommon.fixLen(frommin);
    var tostr = MorpheuzCommon.fixLen(tohr) + MorpheuzCommon.fixLen(tomin);
    var smartStartPoint = new Date(base);
    var early = null;
    var late = null;
    for (var i = 0; i < splitup.length; i++) {
      var teststr1 = smartStartPoint.format("hhmm");
      var smartStartPoint1 = smartStartPoint;
      smartStartPoint = smartStartPoint.addMinutes(MorpheuzCommon.mCommonConst().sampleIntervalMins);
      var teststr2 = smartStartPoint.format("hhmm");
      if (early === null && fromstr >= teststr1 && fromstr <= teststr2) {
        early = MorpheuzCommon.returnAbsoluteMatch(smartStartPoint1, smartStartPoint, fromstr);
      }
      if (late === null && tostr >= teststr1 && tostr <= teststr2) {
        late = MorpheuzCommon.returnAbsoluteMatch(smartStartPoint1, smartStartPoint, tostr);
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
 * Call standard calculateStats but add in canvas control too
 */
function calculateStatsPlusCanvas(base, goneoff, splitup, canvasOverlayConf) {
  var stats = MorpheuzCommon.calculateStats(base, goneoff, splitup);
  if (stats && stats.tbegin && !stats.nosleep) {
    var beginOverlay = {
      verticalLine : {
        name : "begin",
        x : stats.tbegin,
        lineWidth : 1,
        yOffset : 0,
        color : "rgb(255, 149, 0)",
        shadow : false
      }
    };
    canvasOverlayConf.show = true;
    canvasOverlayConf.objects.push(beginOverlay);
  }
  if (stats && stats.tends && !stats.nosleep) {
    var endsStopOverlay = {
      verticalLine : {
        name : "endstop",
        x : stats.tends,
        lineWidth : 1,
        yOffset : 0,
        color : "rgb(255, 149, 0)",
        shadow : false
      }
    };
    canvasOverlayConf.show = true;
    canvasOverlayConf.objects.push(endsStopOverlay);
  }
  return stats;
}

/*
 * Set the tweet reference
 */
function setTweet(rec) {
  $.ajaxSetup({
    scriptCharset : "utf-8",
    contentType : "application/json; charset=utf-8"
  });
  $.getJSON("tweetmysleep.json?v=" + new Date().getTime(), function(data) {
    if (typeof data !== "undefined" && typeof data.tweets !== "undefined") {
      var tweetsForStar = data.tweets.star[rec.stars];
      var ind = Math.floor(Math.random() * tweetsForStar.length);
      var tweet = rec.totalStr + " (" + rec.deepStr + ") - " + tweetsForStar[ind];
      var tweetHref = mConst().twitterWebIntentUrl + encodeURIComponent(tweet);
      $("#tweet").attr("href", tweetHref);
    }
  }).error(function(args) {
    var tweetHref = mConst().twitterWebIntentUrl + encodeURIComponent(mConst().unableToFindTweetText);
    $("#tweet").attr("href", tweetHref);
  });
}

/*
 * Build up the sleep depth strip
 */
function buildStripeChart(splitup) {
  setTimeout(function() {

    // Do the positioning work
    var leftMainPlot = $("#chart1 .jqplot-event-canvas").css("left");
    var widthMainPlot = $("#chart1 .jqplot-event-canvas").css("width");

    var widthFloat = parseFloat(widthMainPlot);

    $('#sleepBar').attr("width", widthFloat);
    $('#sleepBar').css("left", leftMainPlot);
    $('#sleepBar').css("width", widthMainPlot);

    // Fill in the data
    var c = document.getElementById("sleepBar");
    var ctx = c.getContext("2d");
    // Create gradient
    var grd = ctx.createLinearGradient(0, 0, widthFloat, 0);

    grd.addColorStop(0, "#555555");
    for (var i = 0; i < splitup.length; i++) {
      if (splitup[i] === "") {
        continue;
      }
      var position = (i + 1) / 61.0;
      var point = parseInt(splitup[i], 10);
      if (point > MorpheuzCommon.mThres().awakeAbove) {
        grd.addColorStop(position, "#55AAFF");
      } else if (point > MorpheuzCommon.mThres().lightAbove) {
        grd.addColorStop(position, "#0055FF");
      } else if (point == -1 || point == -2) {
        grd.addColorStop(position, "#AAAAAA");
      } else {
        grd.addColorStop(position, "#0000AA");
      }
    }
    grd.addColorStop(1, "#555555");

    // Fill with gradient
    ctx.fillStyle = grd;
    ctx.fillRect(0, 0, widthFloat, 30);

  }, 500);
}

/*
 * Build the environment section
 */
function buildEnvironment(baseDate, pLat, pLong, havePosition) {

  // Only if we have position
  if (!havePosition) {
    return;
  }

  // Work out a colour stop
  function setColourStop(baseDate, targetDate, colour, grd, stops) {
    
    if (targetDate === null || targetDate === "null" || typeof targetDate === "undefined") {
      return;
    }

    var targetDateInt = targetDate.getTime();

    if (isNaN(targetDateInt)) {
      return;
    }
    
    var baseDateInt = baseDate.getTime();

    var diff = targetDateInt - baseDateInt;

    var fsd = MorpheuzCommon.mCommonConst().sampleIntervalMins * 60 * 1000 * mConst().numberOfSamples;

    var offset = diff / fsd;

    if (offset < 0) {
      if (stops.negCloseToZero < offset) {
        stops.negCloseToZero = offset;
        stops.zeroColor = colour;
      }
      return;
    } else if (offset > 1) {
      if (stops.valCloseToOne > offset) {
        stops.valCloseToOne = offset;
        stops.oneColor = colour;
      }
      return;
    }

    grd.addColorStop(offset, colour);

  }

  setTimeout(function() {

    // Do the positioning work
    var leftMainPlot = $("#chart1 .jqplot-event-canvas").css("left");
    var widthMainPlot = $("#chart1 .jqplot-event-canvas").css("width");

    var widthFloat = parseFloat(widthMainPlot);

    $('#sunbar').attr("width", widthFloat);
    $('#sunbar').css("left", leftMainPlot);
    $('#sunbar').css("width", widthMainPlot);

    $('#moonbar').attr("width", widthFloat);
    $('#moonbar').css("left", leftMainPlot);
    $('#moonbar').css("width", widthMainPlot);

    // Work out the sun data
    try {

      var sunTimes = SunCalc.getTimes(baseDate, pLat, pLong);
      var sunTimes2 = SunCalc.getTimes(baseDate.addMinutes(60 * 24), pLat, pLong);

      // Get the canvas
      var cs = document.getElementById("sunbar");
      var ctxs = cs.getContext("2d");

      // Create gradient
      var grds = ctxs.createLinearGradient(0, 0, widthFloat, 0);

      // Define the pallet
      var cSunUp = "#FBF6D9";
      var cSunRise = "#FFE87C";
      var cSunSet = cSunRise;
      var cNadir = "#151B54";
      var cNight = "darkblue"

      // Define end stop data area
      var stops = {
        "negCloseToZero" : -9999,
        "zeroColor" : cNight,
        "oneColor" : cNight,
        "valCloseToOne" : 9999
      }

      // Set the color stops
      setColourStop(baseDate, sunTimes.sunrise, cSunRise, grds, stops);
      setColourStop(baseDate, sunTimes.sunriseEnd, cSunUp, grds, stops);
      setColourStop(baseDate, sunTimes.sunsetStart, cSunSet, grds, stops);
      setColourStop(baseDate, sunTimes.sunset, cNight, grds, stops);
      setColourStop(baseDate, sunTimes.solarNoon, cSunUp, grds, stops);
      setColourStop(baseDate, sunTimes.nadir, cNadir, grds, stops);
      setColourStop(baseDate, sunTimes2.sunrise, cSunRise, grds, stops);
      setColourStop(baseDate, sunTimes2.sunriseEnd, cSunUp, grds, stops);
      setColourStop(baseDate, sunTimes2.sunsetStart, cSunSet, grds, stops);
      setColourStop(baseDate, sunTimes2.sunset, cNight, grds, stops);
      setColourStop(baseDate, sunTimes2.solarNoon, cSunUp, grds, stops);
      setColourStop(baseDate, sunTimes2.nadir, cNadir, grds, stops);
      setColourStop(baseDate, sunTimes.night, cNight, grds, stops);
      setColourStop(baseDate, sunTimes.nightEnd, cNight, grds, stops);
      setColourStop(baseDate, sunTimes2.night, cNight, grds, stops);
      setColourStop(baseDate, sunTimes2.nightEnd, cNight, grds, stops);

      // And the start and end
      grds.addColorStop(0, stops.zeroColor);
      grds.addColorStop(1, stops.oneColor);

      // Fill with gradient
      ctxs.fillStyle = grds;
      ctxs.fillRect(0, 0, widthFloat, 30);

    } catch (err) {
      console.log("Sun calc failed " + err.message);
    }

    // Moon phase
    try {

      var moonIllum = SunCalc.getMoonIllumination(baseDate);
      var phaseNo = Math.round(moonIllum.phase * 8);
      if (phaseNo >= 8) {
        phaseNo = 0;
      }
      $('#moonimg').attr("src", "img/moon-" + phaseNo + ".png");

      // Moon rise and set times
      var moonTimes = SunCalc.getMoonTimes(baseDate, pLat, pLong);
      var moonTimes2 = SunCalc.getMoonTimes(baseDate.addMinutes(60 * 24), pLat, pLong);

      var cm = document.getElementById("moonbar");
      var ctxm = cm.getContext("2d");

      // Create gradient
      var grdm = ctxm.createLinearGradient(0, 0, widthFloat, 0);

      // Moon phase has an effect on brightness
      var cMoonUp = (phaseNo === 3 || phaseNo === 4 || phaseNo === 5) ? "white" : ((phaseNo === 0) ? "black" : "silver");
      var cMoonDown = "black";

      stops = {
        "negCloseToZero" : -9999,
        "zeroColor" : cMoonDown,
        "oneColor" : cMoonDown,
        "valCloseToOne" : 9999
      }

      setColourStop(baseDate, moonTimes.rise, cMoonUp, grdm, stops);
      setColourStop(baseDate, moonTimes2.rise, cMoonUp, grdm, stops);
      setColourStop(baseDate, moonTimes.set, cMoonDown, grdm, stops);
      setColourStop(baseDate, moonTimes2.set, cMoonDown, grdm, stops);
      grdm.addColorStop(0, stops.zeroColor);
      grdm.addColorStop(1, stops.oneColor);

      // Fill with gradient
      ctxm.fillStyle = grdm;
      ctxm.fillRect(0, 0, widthFloat, 30);
      
    } catch (err) {
      console.log("Moon calc failed " + err.message);
    }

  }, 500);
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
  var graphx = getParameterByName("graphx");
  var fromhr = getParameterByName("fromhr");
  var frommin = getParameterByName("frommin");
  var tohr = getParameterByName("tohr");
  var tomin = getParameterByName("tomin");
  var smart = getParameterByName("smart");
  var vers = getParameterByName("vers");
  var goneoff = getParameterByName("goneoff");
  var emailto = getParameterByName("emailto");
  var potoken = getParameterByName("potoken");
  var pouser = getParameterByName("pouser");
  var postat = getParameterByName("postat");
  var swpdo = getParameterByName("swpdo");
  var lifxToken = getParameterByName("lifxtoken");
  var lifxTime = getParameterByName("lifxtime");
  var swpstat = getParameterByName("swpstat");
  var noset = getParameterByName("noset");
  var token = getParameterByName("token");
  var exptime = getParameterByName("exptime");
  var usage = getParameterByName("usage");
  var lazarus = getParameterByName("lazarus");
  var hueip = getParameterByName("hueip");
  var hueusername = getParameterByName("hueuser");
  var hueid = getParameterByName("hueid");
  var ifkey = getParameterByName("ifkey");
  var ifserver = getParameterByName("ifserver");
  var ifstat = getParameterByName("ifstat");
  var age = getParameterByName("age");
  var doemail = getParameterByName("doemail");
  var estat = getParameterByName("estat");
  var snoozes = getParameterByName("zz");
  var latStr = getParameterByName("lat");
  var longStr = getParameterByName("long");
  var fault = getParameterByName("fault");
  var returnTo = getParameterByName("return_to");
  if (returnTo === "") {
    returnTo = "pebblejs://close#";
  }

  var smartOn = smart === "Y";
  var nosetOn = noset === "Y";
  if (nosetOn) {
    $(".noset").hide();
  }
  
  // Provide fault warnings.
  if (fault == 1 || fault == 3) {
    $(".faulta").show();
  }
  if (fault == 2 || fault == 3) {
    $(".faultb").show();
  }

  // Set screen fields
  $("#emailto").val(emailto);
  $("#ptoken").val(potoken);
  $("#puser").val(pouser);
  $("#lifxToken").val(lifxToken);
  $("#lifxTime").val(lifxTime);
  $("#swpdo").prop("checked", swpdo === "Y");
  $("#presult").text(postat);
  $("#swpstat").text(swpstat);
  $("#exptime").text(exptime);
  $("#usage").prop("checked", usage !== "N");
  $("#hueip").val(hueip);
  $("#hueuser").val(hueusername);
  $("#hueid").val(hueid);
  $("#lazarus").prop("checked", lazarus !== "N");
  $("#ifkey").val(ifkey);
  $("#ifserver").val(ifserver);
  $("#ifstat").text(ifstat);
  $("#age").val(age);
  $("#doemail").prop("checked", doemail === "Y");
  $("#estat").text(estat);

  // Set the status bullets for automatic email export
  if (estat === "OK") {
    $("#liemail").addClass("green");
    $("#estat").addClass("green");
  } else if (estat === "" || estat === null || estat === "Disabled") {
    $("#liemail").addClass("blue");
    $("#estat").addClass("blue");
  } else {
    $("#liemail").addClass("red");
    $("#estat").addClass("red");
  }

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

  // Set the status bullets for LIFX
  if (lifxToken === "") {
    $("#lilifx").addClass("blue");
  } else {
    $("#lilifx").addClass("green");
  }

  // Set the status bullets for Hue
  if (hueip === "") {
    $("#lihue").addClass("blue");
  } else {
    $("#lihue").addClass("green");
  }

  // Set the usage bullet to indicate active or not
  if (usage !== "N") {
    $("#liusage").addClass("green");
  } else {
    $("#liusage").addClass("blue");
  }

  // Set the lazarus bullet to indicate active or not
  if (lazarus !== "N") {
    $("#lilazarus").addClass("green");
  } else {
    $("#lilazarus").addClass("blue");
  }

  // Set the status bullets for pushover
  if (ifstat === "OK") {
    $("#liif").addClass("green");
    $("#ifstat").addClass("green");
  } else if (ifstat === "" || ifstat === null || ifstat === "Disabled") {
    $("#liif").addClass("blue");
    $("#ifstat").addClass("blue");
  } else {
    $("#liif").addClass("red");
    $("#ifstat").addClass("red");
  }

  // Any failed exports are automatically opened on load
  $("li.red").removeClass("liclosed").addClass("liopen");

  // Set version
  $("#version").text((parseFloat(vers) / 10).toFixed(1));
  $("#sleep-time").text(new Date(base).format(mConst().displayDateFmt));

  if ((new Date().valueOf()) % 10 === 0) {
    $("#info-message").css("display", "block");
  }

  $(".licollapse > p").click(function() {
    if ($(this).parent().hasClass("liopen")) {
      $(this).parent().removeClass("liopen").addClass("liclosed");
    } else {
      $(this).parent().removeClass("liclosed").addClass("liopen");
    }
  });

  // Show version warning
  if (!nosetOn) {
    setScreenMessageBasedOnVersion(vers);
  }

  // Handle graph or graphx formats
  var splitup = [];
  if (graph === "" && graphx !== "") {
    splitup = splitupFromGraphx(graphx);
  } else if (graph !== "" && graphx === "") {
    splitup = graph.split("!");
  } else {
    console.log("No graph or graphx supplied (or both!)");
  }

  var more = new Array();

  // Build graph data
  buildGraphDataSet(base, splitup, more);

  // Declare canvas overlay which will be populated as we go on
  var canvasOverlayConf = {
    show : false,
    objects : []
  };

  // Build ignore bars
  populateIgnore(base, canvasOverlayConf, splitup, $("#chart1").width());

  // Return start and stop times
  startStopAlarm(smartOn, fromhr, frommin, tohr, tomin, base, canvasOverlayConf, splitup);

  // Return stats
  var out = calculateStatsPlusCanvas(base, goneoff, splitup, canvasOverlayConf);

  // Retain stuff for timed redraw
  document.morpheuzInfo = {
    "splitup" : splitup,
    "base" : new Date(base),
    "pLat" : parseFloat(latStr),
    "pLong" : parseFloat(longStr),
    "havePosition" : (latStr !== "" && longStr !== "")
  }

  // No position, no environment
  if (!document.morpheuzInfo.havePosition) {
    $(".environment").hide();
  }

  // Populate the statistics area
  var snoozes = parseInt(snoozes, 10);
  if (isNaN(snoozes)) {
    snoozes = 0;
  }
  var rec = MorpheuzCommon.buildRecommendationPhrase(age, out, snoozes);
  $("#stats").text(rec.summary);
  $("#ttotal").text(rec.total);
  $("#tawake").text(rec.awake);
  $("#tlight").text(rec.light);
  $("#tdeep").text(rec.deep);
  $("#tignore").text(rec.ignore);
  if (!nosetOn) {
    setTweet(rec);
  }

  // If we have a begin and an end then show this in our 'HealthKit' datapoint
  // section and
  // Make it exportable
  if (out.tbegin !== null && out.tends !== null) {
    $("#swpnodata").hide();
    $("#tstarts").text(out.tbegin.format(mConst().iosDateFormat));
    $("#tends").text(out.tends.format(mConst().iosDateFormat));
    var swpUrl = "https://2hk.swp.io/?source=Morpheuz&starts=" + out.tbegin.format(mConst().swpUrlDate) + "&ends=" + out.tends.format(mConst().swpUrlDate);
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
  var data2 = [ [ "Restless", out.awake ], [ "Light", out.light ], [ "Deep", out.deep ], [ "Ignore", out.ignore ] ];

  // Prepare the graph
  $(document).ready(function() {

    document.plot1 = $.jqplot("chart1", [ more ], {
      grid : {
        background : "transparent",
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
          ticks : [ mConst().chartBottom, MorpheuzCommon.mThres().lightAbove, MorpheuzCommon.mThres().awakeAbove, mConst().chartTop ],
          labelRenderer : $.jqplot.CanvasAxisLabelRenderer,
          label : "Movement",
          min : mConst().chartBottom,
          max : mConst().chartTop,
          labelOptions : {
            textColor : "#1898FF"
          }
        }
      }
    });

    document.plot2 = jQuery.jqplot("chart2", [ data2 ], {
      grid : {
        background : "transparent",
        borderColor : "transparent",
        shadow : false
      },
      seriesColors : [ "#55AAFF", "#0055FF", "#0000AA", "#AAAAAA" ],
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
        show : false,
        location : "e"
      }
    });

    buildStripeChart(document.morpheuzInfo.splitup);

    buildEnvironment(document.morpheuzInfo.base, document.morpheuzInfo.pLat, document.morpheuzInfo.pLong, document.morpheuzInfo.havePosition);

  });

  // Handle the Save and reset option
  $(".save").click(function() {
    var configData = {
      action : "save",
      emailto : safeTrim($("#emailto").val()),
      pouser : safeTrim($("#puser").val()),
      potoken : safeTrim($("#ptoken").val()),
      swpdo : $("#swpdo").is(':checked') ? "Y" : "N",
      usage : $("#usage").is(':checked') ? "Y" : "N",
      lifxtoken : safeTrim($("#lifxToken").val()),
      lifxtime : safeTrim($("#lifxTime").val()),
      hueip : safeTrim($("#hueip").val()),
      hueuser : safeTrim($("#hueuser").val()),
      hueid : safeTrim($("#hueid").val()),
      lazarus : $("#lazarus").is(':checked') ? "Y" : "N",
      testsettings : $("#testsettings").is(':checked') ? "Y" : "N",
      ifkey : safeTrim($("#ifkey").val()),
      ifserver : safeTrim($("#ifserver").val()),
      age : safeTrim($("#age").val()),
      doemail : $("#doemail").is(':checked') ? "Y" : "N"
    };
    document.location = returnTo + encodeURIComponent(JSON.stringify(configData));
  });

  // Send an email containing CSV data
  $("#mail").removeAttr("disabled");
  $("#mail").click(function() {

    // Get email address
    var emailto = $("#emailto").val();

    // Ensure address supplied
    if (emailto === "" || !validateEmail(emailto)) {
      $("#emailSendResult").text(mConst().emailAddressMandatory);
      $("#emailSendResult").addClass("red");
      return;
    }

    // Extract data
    var cpy = MorpheuzCommon.generateCopyLinkData(base, splitup, smartOn, fromhr, frommin, tohr, tomin, goneoff, snoozes);

    var url = mConst().url + vers + ".html" + "?base=" + base + "&fromhr=" + fromhr + "&tohr=" + tohr + "&frommin=" + frommin + "&tomin=" + tomin + "&smart=" + smart + "&vers=" + vers + "&goneoff=" + goneoff + "&token=" + token + "&age=" + age + "&emailto=" + encodeURIComponent(emailto) + "&noset=Y" + "&zz=" + snoozes + "&lat=" + latStr + "&long=" + longStr + "&fault=" + fault;
    if (graph === "") {
      url += "&graphx=" + graphx;
    } else {
      url += "&graph=" + graph;
    }
    
    var email = MorpheuzCommon.buildEmailJsonString(emailto, base, url, cpy);

    // Disable button and put out sending text
    $("#mail").attr("disabled", "disabled");
    $("#emailSendResult").removeClass("red").removeClass("green");
    $("#emailSendResult").text(mConst().sendingEmail);

    // Send to server and await response
    MorpheuzCommon.sendMailViaServer(email, function(stat, resp) {
      if (stat === 1) {
        $("#emailSendResult").addClass("green");
      } else {
        $("#emailSendResult").addClass("red");
      }
      $("#emailSendResult").text(resp);
      $("#mail").removeAttr("disabled");

    })

  });

  // Re-populate stats if age changes
  $("#age").keyup(function() {
    // Populate the statistics area
    var age = $("#age").val();
    var rec = MorpheuzCommon.buildRecommendationPhrase(age, out, snoozes);
    $("#stats").text(rec.summary);
    if (!nosetOn) {
      setTweet(rec);
    }
  });

});
