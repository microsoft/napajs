var assert = require('assert');
var plusNumberTransporter = require('plus-number-transporter');

describe('Test suite for plus-number-transporter', function() {
    it('adds a given value', function() {
        var transporter: any = plusNumberTransporter.createPlusNumberTransporter(3);
        var result: number = plusNumberTransporter.add(transporter, 4);
        assert.equal(result, 7);
    });
})