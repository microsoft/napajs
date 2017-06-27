var assert = require('assert');

assert.equal(typeof tfs, 'undefined');
var tfs = require('tfs');
assert.equal(tfs.runTest(), 'tfs.runTest');

assert.notEqual(typeof tconsole, 'undefined');
assert.equal(tconsole.runTest(), 'tconsole.runTest');

true;