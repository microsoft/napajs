// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

import * as napa from "../lib/index";
import * as assert from 'assert';
import * as path from 'path';
import * as t from './napa-zone/test';

describe('napajs/transport', () => {
    let napaZone = napa.zone.create('zone10');
    describe('TransportContext', () => {
        let tc = napa.transport.createTransportContext();
        let allocator = napa.memory.debugAllocator(napa.memory.crtAllocator);
        it('#saveShared', () => {
            tc.saveShared(allocator);
        });

        let shareable: napa.memory.Shareable = null;
        it('#loadShared', () => {
            shareable = tc.loadShared(allocator.handle);
            assert.deepEqual(shareable.handle, allocator.handle);
        });

        it('#sharedCount', () => {
            assert.equal(shareable.refCount, 3);
        });
    });

    describe('Transportable', () => {
        it('#IsTransportable', () => {
            assert(napa.transport.isTransportable(new t.CanPass(napa.memory.crtAllocator)));
            assert(napa.transport.isTransportable(1));
            assert(napa.transport.isTransportable('hello world'));
            assert(napa.transport.isTransportable([1, 2, 3]));
            assert(napa.transport.isTransportable([1, 2, new t.CanPass(napa.memory.crtAllocator)]));
            assert(napa.transport.isTransportable({ a: 1}));
            assert(napa.transport.isTransportable({ a: 1, b: new t.CanPass(napa.memory.crtAllocator)}));
            assert(napa.transport.isTransportable(() => { return 0; }));
            assert(!napa.transport.isTransportable(new t.CannotPass()));
            assert(!napa.transport.isTransportable([1, new t.CannotPass()]));
            assert(!napa.transport.isTransportable({ a: 1, b: new t.CannotPass()}));
        });
    });

    describe('Marshall/Unmarshall', () => {
        it('@node: simple types', () => {
            t.simpleTypeTransportTest();
        });

        it('@napa: simple types', () => {
            napaZone.execute('./napa-zone/test', "simpleTypeTransportTest");
        }).timeout(3000);

        it('@node: JS transportable', () => {
            t.jsTransportTest();
        });

        it('@napa: JS transportable', () => {
            napaZone.execute('./napa-zone/test', "jsTransportTest");
        });

        it('@node: addon transportable', () => {
            t.addonTransportTest();
        });

        it('@napa: addon transportable', () => {
            napaZone.execute('./napa-zone/test', "addonTransportTest");
        });

        it('@node: function transportable', () => {
            t.functionTransportTest();
        });

        it('@napa: function transportable', () => {
            napaZone.execute('./napa-zone/test', "functionTransportTest");
        });

        it('@node: composite transportable', () => {
            t.compositeTransportTest();
        });

        it('@napa: composite transportable', () => {
            napaZone.execute('./napa-zone/test', "compositeTransportTest");
        });

        it('@node: non-transportable', () => {
            t.nontransportableTest();
        });

        it('@napa: non-transportable', () => {
            napaZone.execute('./napa-zone/test', "nontransportableTest");
        });
    });

    function transportBuiltinObjects() {
        let zoneId: string = 'transport-built-in-test-zone';
        let transportTestZone: napa.zone.Zone = napa.zone.create(zoneId, { workers: 4 });

        /// Construct an expected result string.
        /// constructExpectedResult(5, 5, 255, 0) returns '0,0,0,0,0'
        /// constructExpectedResult(2, 5, 255, 0) returns '0,0,255,255,255'
        /// constructExpectedResult(0, 5, 255, 0) returns '255,255,255,255,255'
        function constructExpectedResult(i: number, size: number, expectedValue: number, defaultValue: number = 0): string {
            const assert = require('assert');
            assert(i >= 0 && size >= i);
            let expected: string = '';
            for (let t: number = 0; t < i; t++) {
                if (t > 0) expected += ',';
                expected += defaultValue.toString();
            }
            for (var t = i; t < size; t++) {
                if (t > 0) expected += ',';
                expected += expectedValue.toString();
            }
            return expected;
        }
        (<any>global).constructExpectedResult = constructExpectedResult;
        transportTestZone.broadcast("global.constructExpectedResult = " + constructExpectedResult.toString());

        it('@node: transport SharedArrayBuffer (SAB)', () => {
            let promises: Array<Promise<any>> = [];
            let sab: SharedArrayBuffer = new SharedArrayBuffer(4);
            for (let i: number = 0; i < 4; i++) {
                promises[i] = transportTestZone.execute((sab, i) => {
                    let ta: Uint8Array = new Uint8Array(sab);
                    ta[i] = 100;
                }, [sab, i]);
            }

            return Promise.all(promises).then((values: Array<napa.zone.Result>) => {
                let ta: Uint8Array = new Uint8Array(sab);
                assert.deepEqual(ta.toString(), '100,100,100,100');
            });
        });

        it('@node: transport composite object of SharedArrayBuffer', () => {
            let sab: SharedArrayBuffer = new SharedArrayBuffer(4);
            let ta1: Uint8Array = new Uint8Array(sab);
            let ta2: Uint8Array = new Uint8Array(sab);
            let obj: Object = { sab: sab, tas: { ta1: ta1, ta2: ta2 }, ta22:ta2 };
            return transportTestZone.execute((obj) => {
                let ta: Uint8Array = new Uint8Array(obj.sab);
                ta[0] = 99;
                obj.tas.ta1[1] = 88;
                obj.tas.ta2[2] = 77;
                obj.ta22[3] = 66;
                }, [obj]).then((result: napa.zone.Result) => {
                    var ta_sab: Uint8Array = new Uint8Array(sab);
                    assert.deepEqual(ta_sab.toString(), '99,88,77,66');
            });
        });

        function recursivelySetElementOfSharedArrayBuffer(zoneId: string, sab: SharedArrayBuffer, i: number, value: number) {
            if (i < 0) return;
            let ta: Uint8Array = new Uint8Array(sab);
            ta[i] = value;

            const assert = require('assert');

            // SharedArrayBuffer shares storage when it is transported,
            // so elements with index > i have been set to {value} by those finished zone.executions.
            let expected: string = (<any>global).constructExpectedResult(i, ta.length, value);
            assert.equal(ta.toString(), expected);

            const napa = require('../lib/index');
            let zone: napa.zone.Zone = (i % 4 < 2) ? napa.zone.get(zoneId) : napa.zone.node;
            zone.execute(
                recursivelySetElementOfSharedArrayBuffer,
                [zoneId, sab, i - 1, value]
            ).then((result: napa.zone.Result) => {
                // SharedArrayBuffer shares storage when it is transported,
                // if i > 0, ta[i - 1] has been set to {value} by the previous zone.execute,
                // so ta.toString() should be larger than {expected} constructed before.
                if (i > 0) assert(ta.toString() > expected);
                else if (i === 0) assert.equal(ta.toString(), expected);
                else assert(false);
            });
        }

        // @node: node -> napa -> napa -> node -> node -> napa -> napa
        it('@node: recursively transport received SharedArrayBuffer (SAB)', () => {
            let size: number = 8;
            let timeout: number = 50;
            let value: number = 255;
            let sab: SharedArrayBuffer = new SharedArrayBuffer(size);
            let ta: Uint8Array = new Uint8Array(sab);
            recursivelySetElementOfSharedArrayBuffer(zoneId, sab, size - 1, value);

            return new Promise((resolve, reject) => {
                setTimeout(() => {
                    // Because SharedArrayBuffer will share storage when it is transported,
                    // once the recursive process finished, all elements of
                    // the original TypeArray (based on SharedArrayBuffer) should have been set to {value}.
                    let expected = (<any>global).constructExpectedResult(0, ta.length, value);
                    assert.equal(ta.toString(), expected);
                    resolve();
                }, timeout);
            });
        });

        function recursivelySetElementOfTypedArray_SAB(zoneId: string, ta: Uint8Array, i: number, value: number) {
            if (i < 0) return;
            ta[i] = value;

            const assert = require('assert');

            // SharedArrayBuffer shares storage when it is transported,
            // so elements with index > i have been set to {value} by those finished zone.executions.
            let expected: string = (<any>global).constructExpectedResult(i, ta.length, value);
            assert.equal(ta.toString(), expected);

            const napa = require('../lib/index');
            let zone: napa.zone.Zone = (i % 4 < 2) ? napa.zone.get(zoneId) : napa.zone.node;
            zone.execute(
                recursivelySetElementOfTypedArray_SAB,
                [zoneId, ta, i - 1, value]
            ).then((result: napa.zone.Result) => {
                // SharedArrayBuffer shares storage when it is transported,
                // if i > 0, ta[i - 1] has been set to {value} by the previous zone.execute,
                // so ta.toString() should be larger than {expected} constructed before.
                if (i > 0) assert(ta.toString() > expected);
                else if (i === 0) assert.equal(ta.toString(), expected);
                else assert(false);
            });
        }

        // @node: node -> napa -> napa -> node -> node -> napa -> napa
        it('@node: recursively transport received TypedArray based on SAB', () => {
            let size: number = 8;
            let timeout: number = 50;
            let value: number = 255;
            let sab: SharedArrayBuffer = new SharedArrayBuffer(size);
            let ta: Uint8Array = new Uint8Array(sab);
            recursivelySetElementOfTypedArray_SAB(zoneId, ta, size - 1, value);

            return new Promise((resolve, reject) => {
                setTimeout(() => {
                    // Because SharedArrayBuffer will share storage when it is transported,
                    // once the recursive process finished, all elements of
                    // the original TypeArray (based on SharedArrayBuffer) should have been set to {value}.
                    let expected: string = (<any>global).constructExpectedResult(0, ta.length, value);
                    assert.equal(ta.toString(), expected);
                    resolve();
                }, timeout);
            });
        });

        function recursivelySetElementOfArrayBuffer(zoneId: string, ab: ArrayBuffer, i: number, value: number) {
            if (i < 0) {
                return;
            }

            let ta: Uint8Array = new Uint8Array(ab);
            ta[i] = value;

            const assert = require('assert');

            // ArrayBuffer's storage will be copied when it is transported.
            // Elements with index > i should all be {value}.
            // They are copied from the previous zone.execution.
            let expected: string = (<any>global).constructExpectedResult(i, ta.length, value);
            assert.equal(ta.toString(), expected);

            const napa = require('../lib/index');
            let zone: napa.zone.Zone = (i % 4 < 2) ? napa.zone.get(zoneId) : napa.zone.node;
            zone.execute(
                recursivelySetElementOfArrayBuffer,
                [zoneId, ab, i - 1, value]
            ).then((result: napa.zone.Result) => {
                // The original TypeArray (based on ArrayBuffer) shouldn't been changed by the just-finished zone.execute.
                assert.equal(ta.toString(), expected);
            });
        }

        // @node: node -> napa -> napa -> node -> node -> napa -> napa
        it('@node: recursively transport received ArrayBuffer (AB)', () => {
            let size: number = 8;
            let timeout: number = 50;
            let value: number = 255;
            let ab: ArrayBuffer = new ArrayBuffer(size);
            let ta: Uint8Array = new Uint8Array(ab);
            recursivelySetElementOfArrayBuffer(zoneId, ab, size - 1, value);

            return new Promise((resolve, reject) => {
                setTimeout(() => {
                    // Except ta[ta-length -1] was set to {value} before the 1st transportation,
                    // the original TypeArray (based on ArrayBuffer) shouldn't been changed by the recursive execution.
                    let expected: string = (<any>global).constructExpectedResult(ta.length - 1, ta.length, value);
                    assert.equal(ta.toString(), expected);
                    resolve();
                }, timeout);
            });
        });

        function recursivelySetElementOfTypeArray_AB(zoneId: string, ta: Uint8Array, i: number, value: number) {
            if (i < 0) {
                return;
            }

            ta[i] = value;

            const assert = require('assert');

            // ArrayBuffer's storage will be copied when it is transported.
            // Elements with index > i should all be {value}.
            // They are copied from the previous zone.execution.
            let expected: string = (<any>global).constructExpectedResult(i, ta.length, value);
            assert.equal(ta.toString(), expected);

            const napa = require('../lib/index');
            let zone: napa.zone.Zone = (i % 4 < 2) ? napa.zone.get(zoneId) : napa.zone.node;
            zone.execute(
                recursivelySetElementOfTypeArray_AB,
                [zoneId, ta, i - 1, value]
            ).then((result: napa.zone.Result) => {
                // The original TypeArray (based on ArrayBuffer) shouldn't been changed by the just-finished zone.execute.
                assert.equal(ta.toString(), expected);
            });
        }

        // @node: node -> napa -> napa -> node -> node -> napa -> napa
        it('@node: recursively transport received TypedArray based on AB', () => {
            let size: number = 8;
            let timeout: number = 50;
            let value: number = 255;
            let ab: ArrayBuffer = new ArrayBuffer(size);
            let ta: Uint8Array = new Uint8Array(ab);
            recursivelySetElementOfTypeArray_AB(zoneId, ta, size - 1, value);

            return new Promise((resolve, reject) => {
                setTimeout(() => {
                    // Except ta[ta-length -1] was set to {value} before the 1st transportation,
                    // the original TypeArray (based on ArrayBuffer) shouldn't been changed by the recursive execution.
                    let expected: string = (<any>global).constructExpectedResult(ta.length - 1, ta.length, value);
                    assert.equal(ta.toString(), expected);
                    resolve();
                }, timeout);
            });
        });
    }

    let builtinTestGroup = 'Transport built-in objects';
    let nodeVersionMajor = parseInt(process.versions.node.split('.')[0]);

    if (nodeVersionMajor >= 9) {
        describe(builtinTestGroup, transportBuiltinObjects);
    } else {
        describe.skip(builtinTestGroup, transportBuiltinObjects);
        require('npmlog').warn(builtinTestGroup, 'This test group is skipped since it requires node v9.0.0 or above.');
    }
});
