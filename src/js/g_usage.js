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

/**
 * Send a usage data
 */
function sendAnonymousUsageData() {
  try {
    var usage = nvl(window.localStorage.getItem("usage"), "Y");
    if (usage !== "Y") {
      console.log("usage opted out");
      return;
    }
    var version = "/v" + window.localStorage.getItem("version");
    console.log("sendAnonymousUsageData: event=" + version);
    var k = ai();
    var apiKey = k.j + k.k + k.f;
    strap_log_event(version, apiKey);
  } catch (err) {
    console.log("strap failed");
  }
}