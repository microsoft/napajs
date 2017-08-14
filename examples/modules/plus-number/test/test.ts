// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

var assert = require('assert');
import * as path from "path";
let plusNumberDir: string = path.resolve(__dirname, '..');
var plusNumber = require(plusNumberDir);
var napa = require('napajs');
var zone = napa.zone.create('zone');

describe('Test suite for plus-number', function() {
    this.timeout(0);

    it('adds a given value', function() {
        var po = plusNumber.createPlusNumber(3);
        var result: number = po.add(4);
        assert.equal(result, 7);
    });

    it('fails with constructor call', function() {
        var failed: boolean = false;
        try {
            var po = new plusNumber.PlusNumber();
        } catch (error) {
            failed = true;
        }
        assert.equal(failed, true);
    });

    it('adds a given value in napa zone', function() {
        return zone.execute((plusNumberDir: string) => {
            var plusNumber = require(plusNumberDir);
            var po = plusNumber.createPlusNumber(3);
            var result: number = po.add(4);
            return result;
        }, [plusNumberDir])
        .then((result: any) => {
            assert.equal(result.value, 7);
        });
    });

    it('adds a given value in node zone', function() {
        return napa.zone.node.execute((plusNumberDir: string) => {
            var plusNumber = require(plusNumberDir);
            var po = plusNumber.createPlusNumber(3);
            var result: number = po.add(4);
            return result;
        }, [plusNumberDir])
        .then((result: any) => {
            assert.equal(result.value, 7);
        });
    });
})
