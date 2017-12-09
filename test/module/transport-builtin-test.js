// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

const assert = require('assert');
const napa = require('../../lib/index');
let zoneKey = 'transport-test-zone';
let zone = napa.zone.create(zoneKey, { workers: 4 });

/// Construct an expected result string.
/// constructExpectedResult(5, 5, 255) returns '0,0,0,0,0'
/// constructExpectedResult(2, 5, 255) returns '0,0,255,255,255'
/// constructExpectedResult(0, 5, 255) returns '255,255,255,255,255'
global.constructExpectedResult =
function constructExpectedResult(i, length, value) {
    const assert = require('assert');
    assert(i >= 0 && length >= i);
    let expected = '';
    for (var t = 0; t < i; t++) {
        if (t > 0) expected += ',';
        expected += '0';
    }
    for (var t = i; t < length; t++) {
        if (t > 0) expected += ',';
        expected += value.toString();
    }
    return expected;
}
zone.broadcast("global.constructExpectedResult = " + constructExpectedResult.toString());

function transportSharedArrayBuffer() {
    var promises = [];
    var sab = new SharedArrayBuffer(4);
    for (var i = 0; i < 4; i++) {
        promises[i] = zone.execute((sab, i) => {
            var ta = new Uint8Array(sab);
            ta[i] = 100;
        }, [sab, i]);
    }

    return Promise.all(promises).then(values => {
        var ta = new Uint8Array(sab);
        assert.deepEqual(ta.toString(), '100,100,100,100');
    });
}
exports.transportSharedArrayBuffer = transportSharedArrayBuffer;

function transportCompositeObjectOfSharedArrayBuffer() {
    let sab = new SharedArrayBuffer(4);
    let ta1 = new Uint8Array(sab);
    let ta2 = new Uint8Array(sab);
    let obj = { sab: sab, tas: { ta1: ta1, ta2: ta2 }, ta22:ta2 };
    return zone.execute((obj) => {
        var ta = new Uint8Array(obj.sab);
        ta[0] = 99;
        obj.tas.ta1[1] = 88;
        obj.tas.ta2[2] = 77;
        obj.ta22[3] = 66;
        }, [obj]).then((result) => {
            var ta_sab = new Uint8Array(sab);
            assert.deepEqual(ta_sab.toString(), '99,88,77,66');
    });
}
exports.transportCompositeObjectOfSharedArrayBuffer = transportCompositeObjectOfSharedArrayBuffer;

function recursivelySetElementOfSharedArrayBuffer(zoneKey, sab, i, value) {
    if (i < 0) return;
    let ta = new Uint8Array(sab);
    ta[i] = value;

    const assert = require('assert');

    // SharedArrayBuffer shares storage when it is transported,
    // so elements with index > i have been set to {value} by those finished zone.executions.
    let expected = global.constructExpectedResult(i, ta.length, value);
    assert.equal(ta.toString(), expected);

    const napa = require('../../lib/index');
    let zone = (i % 4 < 2) ? napa.zone.get(zoneKey) : napa.zone.node;
    zone.execute(
        recursivelySetElementOfSharedArrayBuffer,
        [zoneKey, sab, i - 1, value]
    ).then((result) => {
        // SharedArrayBuffer shares storage when it is transported,
        // if i > 0, ta[i - 1] has been set to {value} by the previous zone.execute,
        // so ta.toString() should be larger than {expected} constructed before.
        if (i > 0) assert(ta.toString() > expected);
        else if (i === 0) assert.equal(ta.toString(), expected);
        else assert(false);
    });
}

function recursivelyTransportSharedArrayBuffer(length, timeout) {
    let value = 255;
    let sab = new SharedArrayBuffer(length);
    let ta = new Uint8Array(sab);
    recursivelySetElementOfSharedArrayBuffer(zoneKey, sab, length - 1, value);

    return new Promise((resolve, reject) => {
        setTimeout(() => {
            // Because SharedArrayBuffer will share storage when it is transported,
            // once the recursive process finished, all elements of
            // the original TypeArray (based on SharedArrayBuffer) should have been set to {value}.
            let expected = global.constructExpectedResult(0, ta.length, value);
            assert.equal(ta.toString(), expected);
            resolve();
        }, timeout);
    });
}
exports.recursivelyTransportSharedArrayBuffer = recursivelyTransportSharedArrayBuffer;


function recursivelySetElementOfTypedArray_SAB(zoneKey, ta, i, value) {
    if (i < 0) return;
    ta[i] = value;

    const assert = require('assert');

    // SharedArrayBuffer shares storage when it is transported,
    // so elements with index > i have been set to {value} by those finished zone.executions.
    let expected = global.constructExpectedResult(i, ta.length, value);
    assert.equal(ta.toString(), expected);

    const napa = require('../../lib/index');
    let zone = (i % 4 < 2) ? napa.zone.get(zoneKey) : napa.zone.node;
    zone.execute(
        recursivelySetElementOfTypedArray_SAB,
        [zoneKey, ta, i - 1, value]
    ).then((result) => {
        // SharedArrayBuffer shares storage when it is transported,
        // if i > 0, ta[i - 1] has been set to {value} by the previous zone.execute,
        // so ta.toString() should be larger than {expected} constructed before.
        if (i > 0) assert(ta.toString() > expected);
        else if (i === 0) assert.equal(ta.toString(), expected);
        else assert(false);
    });
}

function recursivelyTransportTypedArray_SAB(length, timeout) {
    let value = 255;
    let sab = new SharedArrayBuffer(length);
    let ta = new Uint8Array(sab);
    recursivelySetElementOfTypedArray_SAB(zoneKey, ta, length - 1, value);

    return new Promise((resolve, reject) => {
        setTimeout(() => {
            // Because SharedArrayBuffer will share storage when it is transported,
            // once the recursive process finished, all elements of
            // the original TypeArray (based on SharedArrayBuffer) should have been set to {value}.
            let expected = global.constructExpectedResult(0, ta.length, value);
            assert.equal(ta.toString(), expected);
            resolve();
        }, timeout);
    });
}
exports.recursivelyTransportTypedArray_SAB = recursivelyTransportTypedArray_SAB;


function recursivelySetElementOfArrayBuffer(zoneKey, ab, i, value) {
    if (i < 0) {
        return;
    }

    let ta = new Uint8Array(ab);
    ta[i] = value;

    const assert = require('assert');

    // ArrayBuffer's storage will be copied when it is transported.
    // Elements with index > i should all be {value}.
    // They are copied from the previous zone.execution.
    let expected = global.constructExpectedResult(i, ta.length, value);
    assert.equal(ta.toString(), expected);

    const napa = require('../../lib/index');
    let zone = (i % 4 < 2) ? napa.zone.get(zoneKey) : napa.zone.node;
    zone.execute(
        recursivelySetElementOfArrayBuffer,
        [zoneKey, ab, i - 1, value]
    ).then((result) => {
        // The original TypeArray (based on ArrayBuffer) shouldn't been changed by the just-finished zone.execute.
        assert.equal(ta.toString(), expected);
    });
}

function recursivelyTransportArrayBuffer(length, timeout) {
    let value = 255;
    let ab = new ArrayBuffer(length);
    let ta = new Uint8Array(ab);
    recursivelySetElementOfArrayBuffer(zoneKey, ab, length - 1, value);

    return new Promise((resolve, reject) => {
        setTimeout(() => {
            // Except ta[ta-length -1] was set to {value} before the 1st transportation,
            // the original TypeArray (based on ArrayBuffer) shouldn't been changed by the recursive execution.
            let expected = global.constructExpectedResult(ta.length - 1, ta.length, value);
            assert.equal(ta.toString(), expected);
            resolve();
        }, timeout);
    });
}
exports.recursivelyTransportArrayBuffer = recursivelyTransportArrayBuffer;


function recursivelySetElementOfTypeArray_AB(zoneKey, ta, i, value) {
    if (i < 0) {
        return;
    }

    ta[i] = value;

    const assert = require('assert');

    // ArrayBuffer's storage will be copied when it is transported.
    // Elements with index > i should all be {value}.
    // They are copied from the previous zone.execution.
    let expected = global.constructExpectedResult(i, ta.length, value);
    assert.equal(ta.toString(), expected);

    const napa = require('../../lib/index');
    let zone = (i % 4 < 2) ? napa.zone.get(zoneKey) : napa.zone.node;
    zone.execute(
        recursivelySetElementOfTypeArray_AB,
        [zoneKey, ta, i - 1, value]
    ).then((result) => {
        // The original TypeArray (based on ArrayBuffer) shouldn't been changed by the just-finished zone.execute.
        assert.equal(ta.toString(), expected);
    });
}

function recursivelyTransportTypedArray_AB(length, timeout) {
    let value = 255;
    let ab = new ArrayBuffer(length);
    let ta = new Uint8Array(ab);
    recursivelySetElementOfTypeArray_AB(zoneKey, ta, length - 1, value);

    return new Promise((resolve, reject) => {
        setTimeout(() => {
            // Except ta[ta-length -1] was set to {value} before the 1st transportation,
            // the original TypeArray (based on ArrayBuffer) shouldn't been changed by the recursive execution.
            let expected = global.constructExpectedResult(ta.length - 1, ta.length, value);
            assert.equal(ta.toString(), expected);
            resolve();
        }, timeout);
    }); 
}
exports.recursivelyTransportTypedArray_AB = recursivelyTransportTypedArray_AB;
