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

(function() {
  'use strict';

  var MorpheuzUtil = require("./morpheuzUtil");
  var MorpheuzAjax = require("./morpheuzAjax");
  var MorpheuzConfig = require("./morpheuzConfig");

  var MorpheuzHue = {};
  
  /*
   * Turn on the hue lights if configured. Never turn off a light that is
   * already on. If we cannot work out the state of the light leave well alone.
   */
  MorpheuzHue.turnHueLightsOn = function() {

    // Find out config information
    var ip = MorpheuzUtil.getWithDef("hueip", "");
    var username = MorpheuzUtil.getWithDef("hueusername", "");
    var id = MorpheuzUtil.getWithDef("hueid", "");

    // Escape if not configured
    if (ip === "" || username === "" || id === "") {
      console.log("hue control deactivated");
      return;
    }

    // Base url needed to talk with lights
    var baseurl = "http://" + ip + "/api/" + username + "/lights/" + id;

    /*
     * Perform the brightening
     */
    function brightenHue() {
      // On, immediately and dim
      var state1 = {
        "on" : true,
        "bri" : 1,
        "transitiontime" : 0
      };

      // Slowly raise to max brightness
      var state2 = {
        "on" : true,
        "bri" : 254,
        "transitiontime" : 600
      };

      // Set the light on - minimum brightness
      MorpheuzAjax.makeAjaxCall("PUT", baseurl + "/state", MorpheuzConfig.mConst().hueTimeout, JSON.stringify(state1), function(r1) {
        if (typeof r1.status !== 'undefined' && r1.status === 1) {
          console.log("Light on - minimum brightness:" + JSON.stringify(r1));
          setInterval(function() {
            // Set the light to brighten to max over a minute
            MorpheuzAjax.makeAjaxCall("PUT", baseurl + "/state", MorpheuzConfig.mConst().hueTimeout, JSON.stringify(state2), function(r2) {
              if (typeof r2.status !== 'undefined' && r2.status === 1) {
                console.log("Light on - brightening to max:" + JSON.stringify(r2));
              } else {
                console.log("Final state rejected:" + JSON.stringify(r2.errors));
              }
            });
          }, 1000);
        } else {
          console.log("Initial on state, min bright rejected:" + JSON.stringify(r1.errors));
        }
      });
    }

    // Check if the light is on
    MorpheuzAjax.makeAjaxCall("GET", baseurl, MorpheuzConfig.mConst().hueTimeout, "", function(r) {
      if (typeof r.status !== 'undefined' && r.status === 1) {
        if (typeof r.data !== 'undefined' && typeof r.data.state !== 'undefined' && typeof r.data.state.on !== 'undefined') {
          if (!r.data.state.on) {
            brightenHue();
          } else {
            console.log("light is on. Leave alone. Don't plunge into darkness");
          }
        } else {
          console.log("No valid hue response");
        }
      } else {
        console.log("No response:" + JSON.stringify(r.errors));
      }
    });
  };

  module.exports = MorpheuzHue;

}());
