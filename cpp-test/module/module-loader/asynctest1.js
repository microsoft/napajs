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
    assert.equal(value, 3);

    now = async.now();
    assert.equal(now, 6);
});

async.increaseSync(3, (value) => {});

true;