now = async.now();
assert.equal(now, 6);

async.increaseSync(3, (value) => {
    assert.equal(value, 9);

    now = async.now();
    assert.equal(now, 12);
});

async.increaseSync(3, (value) => {});

true;
