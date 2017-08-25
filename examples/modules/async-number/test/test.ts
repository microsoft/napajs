// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

let assert = require('assert');
let asyncNumber = require('..');
let napa = require('napajs');
let zone = napa.zone.create('zone');

describe('Test suite for async-number', function() {
    this.timeout(0);

    it('change number asynchronously on separate thread', () => {
        let now = asyncNumber.now();
        assert.equal(now, 0);

        asyncNumber.increase(3, (value: number) => {
            // This must be called after the last statement of *it* block is executed.
            assert(value == 3 || value == 6);

            now = asyncNumber.now();
            assert.equal(now, 6);
        });

        asyncNumber.increaseSync(3, (value: number) => {} );
    });

    it('change number synchronously on current thread', () => {
        let now = asyncNumber.now();
        assert.equal(now, 6);

        asyncNumber.increaseSync(3, (value: number) => {
            // This must be called after the last statement of *it* block is executed.
            assert.equal(value, 9);

            now = asyncNumber.now();
            assert.equal(now, 12);
        });

        now = asyncNumber.now();
        // 'now' should be 9.
        assert.equal(now, 9);

        asyncNumber.increaseSync(3, (value: number) => {} );
    });

    it('change number asynchronously on separate thread in napa zone', () => {
        return zone.execute(() => {
            let assert = require('assert');
            let asyncNumber = require('..');
            let now = asyncNumber.now();
            assert.equal(now, 0);

            asyncNumber.increase(3, (value: number) => {
                // This must be called after the last statement of *it* block is executed.
                assert(value == 3 || value == 6);

                now = asyncNumber.now();
                assert.equal(now, 6);
            });

            asyncNumber.increaseSync(3, (value: number) => {} );
        }, []);
    });

    it('change number synchronously on current thread in napa zone', () => {
        zone.execute(() => {
            let assert = require('assert');
            let asyncNumber = require('..');
            let now = asyncNumber.now();
            assert.equal(now, 0);

            asyncNumber.increaseSync(3, (value: number) => {
                // This must be called after the last statement of *it* block is executed.
                assert.equal(value, 3);

                now = asyncNumber.now();
                assert.equal(now, 6);
            });

            now = asyncNumber.now();
            // 'now' should be 3.
            assert.equal(now, 3);
            asyncNumber.increaseSync(3, (value: number) => {} );
            return 1;
        }, []);
    });
})
