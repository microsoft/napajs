var assert = require('assert');
var async = require('./async');

var now = async.now();
assert.equal(now, 0);

var promise = new Promise((resolve) => {
    async.increase(3, (value) => {
        resolve(value);
    });
});

promise.then((value) => {
    assert.equal(value, 4);
});

now = async.now();
assert(now == 0 || now == 3);

true;