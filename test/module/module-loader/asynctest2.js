now = async.now();
assert.equal(now, 4);

promise = new Promise((resolve) => {
    async.increase(3, (value) => {
        resolve(value);
    });
});

promise.then((value) => {
    assert.equal(value, 8);
});

now = async.now();
assert(now == 4 || now == 7);

true;
