var assert = require('assert');

assert(1 == 1);

const obj1 = { a : { b : 1 } };
const obj2 = { a : { b : 2 } };
const obj3 = { a : { b : 1 } };
const obj4 = Object.create(obj1);

assert.deepEqual(obj1, obj1);

assert.notDeepEqual(obj1, obj2);

assert.deepEqual(obj1, obj3);

assert.notDeepEqual(obj1, obj4);

assert.deepEqual({a:1}, {a:'1'});

assert.notStrictEqual({a:1}, {a:'1'});

assert.equal(1, 1);

assert.equal(1, '1');

assert.deepEqual(1, '1');

assert.deepEqual(1, 1);

assert.notStrictEqual(1, '1');

assert.notEqual(1, 2);

assert.notEqual({a: {b: 1}}, {a: {b: 1}});

assert.ifError(0);

assert.throws(() => { assert.ifError(1);});

assert.throws(() => { assert.ifError('error');});

assert.throws(() => { assert.ifError(new Error());});

assert.throws(() => { assert.fail(1, 2, undefined, '>'); });

assert.ok(true);

assert.ok(1);

assert.throws(() => { assert.ok(false); });

assert.throws(() => { assert.ok(0); });

assert.throws(() => {
    assert.doesNotThrow(() => { throw new TypeError('Wrong value'); }, TypeError);
});

true;
