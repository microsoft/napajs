// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

let assert = require('assert');
import * as path from "path";
let helloWorldDir: string = path.resolve(__dirname, '..');
let helloWorld = require(helloWorldDir);
let napa = require('napajs');
let zone = napa.zone.create('zone');

describe('Test suite for hello-word', function() {
    this.timeout(0);

    it('prints the string "world"', function() {
        let result: string = helloWorld.hello();
        assert.equal(result, 'world');
    });

    it('prints the string "world" in napa zone', function() {
        return zone.execute((helloWorldDir: string) => {
            let helloWorld = require(helloWorldDir);
            let result: string = helloWorld.hello();
            return result;
        }, [helloWorldDir]).then((result : any) => {
            assert.equal(result.value, 'world');
        });
    });
})
