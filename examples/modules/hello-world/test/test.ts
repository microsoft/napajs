// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

var assert = require('assert');
import * as path from "path";
let helloWorldDir: string = path.resolve(__dirname, '..');
var helloWorld = require(helloWorldDir);
var napa = require('napajs');
var zone = napa.zone.create('zone');

describe('Test suite for hello-word', function() {
    this.timeout(0);

    it('prints the string "world"', function() {
        var result: string = helloWorld.hello();
        assert.equal(result, 'world');
    });

    it('prints the string "world" in napa zone', function() {
        return zone.execute((helloWorldDir: string) => {
            var helloWorld = require(helloWorldDir);
            var result: string = helloWorld.hello();
            return result;
        }, [helloWorldDir]).then((result : any) => {
            assert.equal(result.value, 'world');
        });
    });
})
