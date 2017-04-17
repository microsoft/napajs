var assert = require('assert');

assert.equal(global.Math.floor(1.1), 1);

var test = require('./globaltest-module');
global.fromGlobal = 'fromGlobal';
assert.equal(test.run(), 'fromGlobal');
assert.equal(global.toGlobal, 'toGlobal');

true;