var assert = require('assert');
var tty = require('tty');

var result = process.binding('tty_wrap').guessHandleType(1);
assert.equal(result, 'UNKNOWN');

assert.equal(tty.isatty(3), false);

true;
