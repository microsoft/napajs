var assert = require('assert');
var fs = require('fs');

assert.ifError(fs.existsSync('file-not-exist.json'));

assert.ok(fs.existsSync('fstest.json'));

var content = JSON.parse(fs.readFileSync('fstest.json'));
assert.equal(content.query, 'query');
assert.equal(content.document, 'document');

fs.mkdirSync('fstest-dir');
fs.mkdirSync('fstest-dir/1');
fs.mkdirSync('fstest-dir/2');
fs.writeFileSync('fstest-dir/3.dat', '3.dat');
var names = fs.readdirSync('fstest-dir');
assert.deepEqual(names, ['1', '2', '3.dat']);

require('./tests/sub/fstestsub');
