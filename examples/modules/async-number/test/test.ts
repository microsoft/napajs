// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

var assert = require('assert');
var asyncNumber = require('async-number');

describe('Test suite for async-number',
         function() {
             it('change number asynchronously on separate thread',
                (done: () => void) => {
                    let now = asyncNumber.now();
                    assert.equal(now, 0);

                    asyncNumber.increase(3,
                                         (value: number) => {
                                             // This must be called after the last statement of *it* block is executed.
                                             assert(value == 3 || value == 6);

                                             now = asyncNumber.now();
                                             assert.equal(now, 6);

                                             done();
                                         });

                    asyncNumber.increaseSync(3, (value: number) => {});
                });

             it('change number synchronously on current thread',
                (done: () => void) => {
                    let now = asyncNumber.now();
                    assert.equal(now, 0);

                    asyncNumber.increaseSync(3,
                                             (value: number) => {
                                                 // This must be called after the last statement of *it* block is
                                                 // executed.
                                                 assert.equal(value, 3);

                                                 now = asyncNumber.now();
                                                 assert.equal(now, 6);

                                                 done();
                                             });

                    now = asyncNumber.now();
                    // 'now' should be 3.
                    assert.equal(now, 3);

                    asyncNumber.increaseSync(3, (value: number) => {});
                });
         })
