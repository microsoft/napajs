// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

import * as napa from "../lib/index";
import * as assert from 'assert';

// spin-wait for the specific time, in milliseconds.
function spinWait(time: number) {
    let now = Date.now();
    while (Date.now() - now < time) { }
}

describe('napajs/store', function () {
    this.timeout(0);

    let napaZone = napa.zone.create('zone-for-sync-test');
    napaZone.broadcast(spinWait.toString());
    napaZone.broadcast("require('../lib/index')");

    it('@node: sync.Lock - create a lock', () => {
        let succeed = false;
        try {
            let lock = new napa.sync.Lock();
            succeed = true;
        }
        catch (error) {
        }
        assert(succeed);
    });

    it('@node: sync.Lock - guard single thread sync execution', () => {
        let succeed = false;
        let index = 0;
        try {
            let lock = new napa.sync.Lock();
            lock.guardSync(function () {
                spinWait(100);
                assert(index == 0);
                index++;
            });
            lock.guardSync(function () {
                spinWait(100);
                assert(index == 1);
                index++;
            });
            assert(index == 2);
            succeed = true;
        }
        catch (error) {
        }
        assert(succeed);
    });

    it('@node: sync.Lock - multi thread sync execution (not using lock)', () => {

        // If not using lock, the store.set() in second execute() function should start before the first complete.
        let succeed = false;
        try {
            let lock = new napa.sync.Lock();
            // We use napa.store to verify the result
            napa.store.getOrCreate('store-for-sync-test').set('k1', 0);

            let exe1 = napaZone.execute(function () {
                (<any>global).spinWait(100);
                var napa = require('../lib/index');
                napa.store.get('store-for-sync-test').set('k1', 1);
            });

            spinWait(50);
            let exe2 = napaZone.execute(function () {
                var assert = require('assert');
                var napa = require('../lib/index');
                assert(napa.store.get('store-for-sync-test').get('k1') == 0);
            });

            return Promise.all([exe1, exe2]).then(function () {
                assert(napa.store.get('store-for-sync-test').get('k1') == 1);
            });
        }
        catch (error) {
        }
    });
    
    it('@node: sync.Lock - multi thread sync execution (using lock)', () => {

        // If guard by lock, the store.set() in second execute() function should start after the first complete.
        let succeed = false;
        try {
            let lock = new napa.sync.Lock();
            // We use napa.store to verify the result
            napa.store.getOrCreate('store-for-sync-test').set('k1', 0);

            let exe1 = napaZone.execute(function (lock) {
                lock.guardSync(function () {
                    (<any>global).spinWait(100);
                    var napa = require('../lib/index');
                    napa.store.get('store-for-sync-test').set('k1', 1);
                });
            }, [lock]);

            spinWait(50);
            let exe2 = napaZone.execute(function (lock) {
                lock.guard(function () {
                    var assert = require('assert');
                    var napa = require('../lib/index');
                    assert(napa.store.get('store-for-sync-test').get('k1') == 1);
                });
            }, [lock]);

            return Promise.all([exe1, exe2]).then(function () {
                assert(napa.store.get('store-for-sync-test').get('k1') == 1);
            });
        }
        catch (error) {
        }
    });

});
