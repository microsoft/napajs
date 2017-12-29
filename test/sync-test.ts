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

    it('@node: sync.Lock - parameters passing', () => {
        let succeed = false;
        try {
            let lock = napa.sync.createLock();
            lock.guardSync((a, b, c) => {
                assert.strictEqual(a, 123);
                assert.strictEqual(b, '456');
                assert.strictEqual(c, undefined);
            }, [123, '456']);
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

        // If not using lock, the store.set() in second execute() function should start before the first complete.
        let succeed = false;
        try {
            let lock = napa.sync.createLock();
            // We use napa.store to verify the result
            let store = napa.store.create('store-for-sync-test-1');
            store.set('before-wait', 0);
            store.set('after-wait', 0);

            let exe1 = napaZone.execute(function () {
                let napa = require('../lib/index');
                let store = napa.store.get('store-for-sync-test-1');
                store.set('before-wait', 1);
                (<any>global).spinWait(500);
                store.set('after-wait', 1);
            });

            let exe2 = napaZone.execute(function () {
                let assert = require('assert');
                let napa = require('../lib/index');
                let store = napa.store.get('store-for-sync-test-1');
                
                (<any>global).spinWait(100);
                assert.equal(store.get('before-wait'), 1);
                assert.equal(store.get('after-wait'), 0);
            });

            return Promise.all([exe1, exe2]).then(function () {
                assert.equal(store.get('before-wait'), 1);
                assert.equal(store.get('after-wait'), 1);
            });
        }
        catch (error) {
        }
    }).timeout(5000);
    
    it('@napa: sync.Lock - multi thread sync execution (using lock)', () => {
        let napaZone = napa.zone.create('zone-for-sync-test-2', { workers: 2 });
        napaZone.broadcast(spinWait.toString());

        // If guard by lock, the store.set() in second execute() function should start after the first complete.
        let succeed = false;
        try {
            let lock = napa.sync.createLock();
            // We use napa.store to verify the result
            let store = napa.store.create('store-for-sync-test-2');
            store.set('before-wait', 0);
            store.set('after-wait', 0);

            let exe1 = napaZone.execute(function (lock) {
                let napa = require('../lib/index');
                let store = napa.store.get('store-for-sync-test-2');
                
                lock.guardSync(function () {
                    store.set('before-wait', 1);
                    (<any>global).spinWait(500);
                    store.set('after-wait', 1);
                });
            }, [lock]);

            let exe2 = napaZone.execute(function (lock) {
                let assert = require('assert');
                let napa = require('../lib/index');
                let store = napa.store.get('store-for-sync-test-2');
                
                (<any>global).spinWait(100);

                lock.guardSync(function () {
                    assert.equal(store.get('before-wait'), 1);
                    assert.equal(store.get('after-wait'), 1);
                });
            }, [lock]);

            return Promise.all([exe1, exe2]).then(function () {
                assert.equal(store.get('before-wait'), 1);
                assert.equal(store.get('after-wait'), 1);
            });
        }
        catch (error) {
        }
    }).timeout(5000);

});
