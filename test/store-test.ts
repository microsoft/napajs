// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

import * as napa from "../lib/index";
import * as assert from 'assert';
import * as path from 'path';

describe('napajs/store', function () {
    let napaZone = napa.zone.create('zone6');
    let store1 = napa.store.create('store1');
    it('@node: store.create - succeed', () => {
        assert(store1 != null);
        assert.equal(store1.id, 'store1');
        assert.equal(store1.size, 0);
    });

    it('@node: store.create - already exists', () => {
        let succeed = false;
        try {
            let store = napa.store.create('store1');
            succeed = true;
        }
        catch (error) {
        }
        assert(!succeed);
    });

    let store2CreationComplete: Promise<napa.zone.Result>;

    it('@napa: store.getOrCreate', () => {
        store2CreationComplete = napaZone.execute('./napa-zone/test', "getOrCreateStoreTest");
    });

    it('@node: store.get', async () => {
        let store = napa.store.get('store1');
        assert.equal(store.id, store1.id);

        // Store created from napa zone.
        await store2CreationComplete;
        let store2 = napa.store.get('store2');
        assert.equal(store2.id, 'store2');
    });

    it('@napa: store.get', async () => {
        await napaZone.execute('./napa-zone/test', "getStoreTest");
    });

    it('simple types: set in node, get in node', () => {
        store1.set('a', 1);
        assert.equal(store1.get('a'), 1);
    });

    it('simple types: set in node, get in napa', async () => {
        store1.set('b', 'hi');
        await napaZone.execute('./napa-zone/test', "storeVerifyGet", ['store1', 'b', 'hi']);
    });

    it('simple types: set in napa, get in napa', async () => {
        await napaZone.execute('./napa-zone/test', "storeSet", ['store1', 'c', 1]);
        await napaZone.execute('./napa-zone/test', "storeVerifyGet", ['store1', 'c', 1]);
    });

    it('simple types: set in napa, get in node', async () => {
        await napaZone.execute('./napa-zone/test', "storeSet", ['store1', 'd', { a: 1, b: 1}]);
        assert.deepEqual(store1.get('d'), {
            a: 1,
            b: 1
        });
    });

    let unicodeStr = "中文 español deutsch English हिन्दी العربية português বাংলা русский 日本語 ਪੰਜਾਬੀ 한국어 தமிழ் עברית";
    let store2 = napa.store.create('store2');

    it('unicode string: set in node, get in node', () => {
        store2.set('a', unicodeStr);
        assert.equal(store2.get('a'), unicodeStr);
    });

    it('unicode string: set in node, get in napa', async () => {
        store2.set('b', unicodeStr);
        await napaZone.execute('./napa-zone/test', "storeVerifyGet", ['store2', 'b', unicodeStr]);
    });

    it('unicode string: set in napa, get in napa', async () => {
        await napaZone.execute('./napa-zone/test', "storeSet", ['store2', 'c', unicodeStr]);
        await napaZone.execute('./napa-zone/test', "storeVerifyGet", ['store2', 'c', unicodeStr]);
    });

    it('unicode string: set in napa, get in node', async () => {
        await napaZone.execute('./napa-zone/test', "storeSet", ['store2', 'd', { a: 1, b: unicodeStr}]);
        assert.deepEqual(store2.get('d'), {
            a: 1,
            b: unicodeStr
        });
    });

    it('transportable types: set in node, get in node', () => {
        store1.set('a', napa.memory.crtAllocator);
        assert.deepEqual(store1.get('a'), napa.memory.crtAllocator);
    });

    it('transportable types: set in node, get in napa', async () => {
        store1.set('b', napa.memory.defaultAllocator);
        await napaZone.execute('./napa-zone/test', "storeVerifyGet", ['store1', 'b', napa.memory.defaultAllocator]);
    });

    it('transportable types: set in napa, get in napa', async () => {
        // We have to compare handle in this case, since napa.memory.defaultAllocator retrieved from napa zone will have 2+ refCount.
        await napaZone.execute('./napa-zone/test', "storeSet", ['store1', 'e', napa.memory.defaultAllocator]);
        await napaZone.execute('./napa-zone/test', "storeGetCompareHandle", ['store1', 'e', napa.memory.defaultAllocator.handle]);
    });

    it('transportable types: set in napa, get in node', async () => {
        let debugAllocator = napa.memory.debugAllocator(napa.memory.defaultAllocator);
        await napaZone.execute('./napa-zone/test', "storeSet", ['store1', 'f', debugAllocator]);
        assert.deepEqual(store1.get('f'), debugAllocator);
    });

    it('function type: set in node, get in node', () => {
        store1.set('g', () => { return 0; });
        assert.equal(store1.get('g').toString(), (() => { return 0; }).toString());
    });

    it('function type: set in node, get in napa', async () => {
        store1.set('h', () => { return 0; });
        await napaZone.execute('./napa-zone/test', "storeVerifyGet", ['store1', 'h', () => { return 0; }]);
    });

    it('function type: set in napa, get in napa', async () => {
        await napaZone.execute('./napa-zone/test', "storeSet", ['store1', 'i', () => { return 0; }]);
        await napaZone.execute('./napa-zone/test', "storeVerifyGet", ['store1', 'i', () => { return 0; }]);
    });

    it('function type: set in napa, get in node', async () => {
        await napaZone.execute('./napa-zone/test', "storeSet", ['store1', 'j', () => { return 0; }]);
        assert.deepEqual(store1.get('j').toString(), (() => { return 0; }).toString());
    });

    it('delete in node, check in node', () => {
        assert(store1.has('a'));
        store1.delete('a');
        assert(!store1.has('a'));
        assert(store1.get('a') === undefined);
        store1.delete('not-exist');
    });

    it('delete in node, check in napa', async () => {
        assert(store1.has('b'));
        store1.delete('b');
        await napaZone.execute('./napa-zone/test', "storeVerifyNotExist", ['store1', 'b']);
    });

    it('delete in napa, check in napa', async () => {
        await napaZone.execute('./napa-zone/test', "storeDelete", ['store1', 'c']);
        await napaZone.execute('./napa-zone/test', "storeVerifyNotExist", ['store1', 'c']);
    })

    it('delete in napa, check in node', async () => {
        await napaZone.execute('./napa-zone/test', "storeDelete", ['store1', 'd']);
        assert(!store1.has('d'));
        assert(store1.get('d') === undefined);
    });

    it('size', () => {
        // set 'a', 'b', 'c', 'd', 'a', 'b', 'e', 'f', 'g', 'h', 'i', 'j'.
        // delete 'a', 'b', 'c', 'd'
        assert.equal(store1.size, 6);
    });
});
