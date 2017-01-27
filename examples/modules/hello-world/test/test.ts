var assert = require('assert');
var helloWorld = require('hello-world');

describe('Test suite for hello-word', function() {
    it('prints the string "world"', function() {
        var result = helloWorld.hello();
        assert.equal(result, 'world');
    });
})
