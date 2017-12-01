// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

import * as napa from "../lib/index";
import * as assert from 'assert';

// spin-wait for the specific time, in milliseconds.
function spinWait(time: number) {
    let now = Date.now();
    while (Date.now() - now < time) {}
}

describe('napajs/sync', function () {

    it('@node: sync.Lock - create a lock', () => {
        let succeed = false;
        try {
            let lock = napa.sync.createLock();
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
            let lock = napa.sync.createLock();
            lock.guardSync(function () {
                spinWait(100);
                assert.equal(index, 0);
                index++;
            });
            lock.guardSync(function () {
                spinWait(100);
                assert.equal(index, 1);
                index++;
            });
            assert.equal(index, 2);
            succeed = true;
        }
        catch (error) {
        }
        assert(succeed);
    });

    it('@napa: sync.Lock - multi thread sync execution (not using lock)', () => {
        let napaZone = napa.zone.create('zone-for-sync-test-1', { workers: 2 });
        napaZone.broadcast(spinWait.toString());
        napaZone.broadcast("require('../lib/index')");

        // If not using lock, the store.set() in second execute() function should start before the first complete.
        let succeed = false;
        try {
            let lock = napa.sync.createLock();
            // We use napa.store to verify the result
            napa.store.getOrCreate('store-for-sync-test-1').set('before-wait', 0);
            napa.store.getOrCreate('store-for-sync-test-1').set('after-wait', 0);

            let exe1 = napaZone.execute(function () {
                var napa = require('../lib/index');

                napa.store.get('store-for-sync-test-1').set('before-wait', 1);
                (<any>global).spinWait(100);
                napa.store.get('store-for-sync-test-1').set('after-wait', 1);
            });

            let exe2 = napaZone.execute(function () {
                var assert = require('assert');
                var napa = require('../lib/index');
                
                (<any>global).spinWait(50);
                assert.equal(napa.store.get('store-for-sync-test-1').get('before-wait'), 1);
                assert.equal(napa.store.get('store-for-sync-test-1').get('after-wait'), 0);
            });

            return Promise.all([exe1, exe2]).then(function () {
                assert.equal(napa.store.get('store-for-sync-test-1').get('before-wait'), 1);
                assert.equal(napa.store.get('store-for-sync-test-1').get('after-wait'), 1);
            });
        }
        catch (error) {
        }
    });
    
    it('@napa: sync.Lock - multi thread sync execution (using lock)', () => {
        let napaZone = napa.zone.create('zone-for-sync-test-2', { workers: 2 });
        napaZone.broadcast(spinWait.toString());
        napaZone.broadcast("require('../lib/index')");


        // If guard by lock, the store.set() in second execute() function should start after the first complete.
        let succeed = false;
        try {
            let lock = napa.sync.createLock();
            // We use napa.store to verify the result
            napa.store.getOrCreate('store-for-sync-test-2').set('before-wait', 0);
            napa.store.getOrCreate('store-for-sync-test-2').set('after-wait', 0);

            let exe1 = napaZone.execute(function (lock) {
                var napa = require('../lib/index');

                lock.guardSync(function () {
                    napa.store.get('store-for-sync-test-2').set('before-wait', 1);
                    (<any>global).spinWait(100);
                    napa.store.get('store-for-sync-test-2').set('after-wait', 1);
                });
            }, [lock]);

            let exe2 = napaZone.execute(function (lock) {
                var assert = require('assert');
                var napa = require('../lib/index');

                (<any>global).spinWait(50);

                lock.guardSync(function () {
                    assert.equal(napa.store.get('store-for-sync-test-2').get('before-wait'), 1);
                    assert.equal(napa.store.get('store-for-sync-test-2').get('after-wait'), 1);
                });
            }, [lock]);

            return Promise.all([exe1, exe2]).then(function () {
                assert.equal(napa.store.get('store-for-sync-test-2').get('before-wait'), 1);
                assert.equal(napa.store.get('store-for-sync-test-2').get('after-wait'), 1);
            });
        }
        catch (error) {
        }
    });

});
