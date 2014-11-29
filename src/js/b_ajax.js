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
 * Standard Post Ajax call routine
 */
function makePostAjaxCall(url, msg, resp) {
  var req = new XMLHttpRequest();
  req.open('POST', url, true);
  req.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  req.setRequestHeader('Content-length', msg.length);
  req.setRequestHeader('Connection', 'close');
  req.onload = function() {
    if (req.readyState === 4 && req.status === 200) {
      var result = JSON.parse(req.responseText);
      resp(result);
    } else if (req.readyState === 4 && (req.status >= 400 && req.status < 500)) {
      var result = JSON.parse(req.responseText);
      resp(result);
    } else if (req.readyState === 4 && (req.status === 500)) {
      resp({ "status": 0, "errors": ["500 error"]});
    }
  };
  req.send(msg);
}

/*
 * Standard Get Ajax call routine
 */
function makeGetAjaxCall(url, resp) {
  var req = new XMLHttpRequest();
  req.open('GET', url, true);
  req.onload = function() {
    if (req.readyState === 4 && req.status === 200) {
      resp({ "status": 1});
    } else if (req.readyState === 4 && (req.status >= 400 && req.status < 500)) {
      resp({ "status": 0, "errors": ["4nn error"]});
    } else if (req.readyState === 4 && (req.status === 500)) {
      resp({ "status": 0, "errors": ["500 error"]});
    }
  };
  req.send();
}

