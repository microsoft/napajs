// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

var assert = require('assert');
var helloWorld = require('..');

describe('Test suite for hello-word', function() {
    it('prints the string "world"', function() {
        var result: string = helloWorld.hello();
        assert.equal(result, 'world');
    });
})
