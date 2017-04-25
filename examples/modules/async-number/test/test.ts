var assert = require('assert');
var asyncNumber = require('async-number');

describe('Test suite for async-number', function() {
    it('change number asynchronously', function(done) {
        let now = asyncNumber.now();
        assert.equal(now, 0);

        asyncNumber.increase(3, (value: number) => {
            // This must be called after the last statement of *it* block is executed.
            assert.equal(value, 4);
            done();
        });

        now = asyncNumber.now();
        // 'now' should not 4.
        assert(now == 0 || now == 3);
    });
})