var assert = require('assert');
var plusNumber = require('plus-number');

describe('Test suite for plus-number', function() {
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
})
