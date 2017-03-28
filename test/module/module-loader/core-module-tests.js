if (typeof fs != 'undefined') throw newError();
var fs = require('fs');
if (fs.runTest() != 'fs.runTest') throw newError();

if (typeof console == undefined) throw newError();
if (console.runTest() != 'console.runTest') throw newError();

var assert = require('assert');
if (assert.runTest() != 'assert.runTest') throw newError();

true;