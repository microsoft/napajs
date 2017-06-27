var assert = require('assert');
var os = require('os');

assert(os.type == "Windows_NT" || os.type == "Darwin" || os.type == "Linux");

true;