// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

let assert = require('assert');
let plusNumber = require('..');
let napa = require('napajs');
let zone = napa.zone.create('zone');

describe('Test suite for plus-number', function() {
    this.timeout(0);

    it('adds a given value', function() {
        let po = plusNumber.createPlusNumber(3);
        let result: number = po.add(4);
        assert.equal(result, 7);
    });

    it('fails with constructor call', function() {
        let failed: boolean = false;
        try {
            let po = new plusNumber.PlusNumber();
        } catch (error) {
            failed = true;
        }
        assert.equal(failed, true);
    });

    it('adds a given value in napa zone', function() {
        return zone.execute(() => {
            let plusNumber = require('..');
            let po = plusNumber.createPlusNumber(3);
            let result: number = po.add(4);
            return result;
        }, [])
        .then((result: any) => {
            assert.equal(result.value, 7);
        });
    });

    it('adds a given value in node zone', function() {
        return napa.zone.node.execute(() => {
            let plusNumber = require('..');
            let po = plusNumber.createPlusNumber(3);
            let result: number = po.add(4);
            return result;
        }, [])
        .then((result: any) => {
            assert.equal(result.value, 7);
        });
    });
})
