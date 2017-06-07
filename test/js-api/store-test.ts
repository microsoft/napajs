import * as napa from "napajs";
import * as assert from 'assert';
import * as path from 'path';

let NAPA_ZONE_TEST_MODULE = path.resolve(__dirname, 'napa-zone/test');

describe('napajs/store', () => {
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

    it('@napa: store.getOrCreate', () => {
        napaZone.executeSync(NAPA_ZONE_TEST_MODULE, "getOrCreateStoreTest", []);
    });

    it('@node: store.get', () => {
        let store = napa.store.get('store1');
        assert.equal(store.id, store1.id);

        // Store created from napa zone.
        let store2 = napa.store.get('store2');
        assert.equal(store2.id, 'store2');
    });

    it('@napa: store.get', () => {
        napaZone.executeSync(NAPA_ZONE_TEST_MODULE, "getStoreTest", []);
    });

    it('simple types: set in node, get in node', () => {
        store1.set('a', 1);
        assert.equal(store1.get('a'), 1);
    });

    it('simple types: set in node, get in napa', () => {
        store1.set('b', 'hi');
        napaZone.executeSync(NAPA_ZONE_TEST_MODULE, "storeVerifyGet", ['store1', 'b', 'hi']);
    });

    it('simple types: set in napa, get in napa', () => {
        napaZone.executeSync(NAPA_ZONE_TEST_MODULE, "storeSet", ['store1', 'c', 1]);
        napaZone.executeSync(NAPA_ZONE_TEST_MODULE, "storeVerifyGet", ['store1', 'c', 1]);
    });

    it('simple types: set in napa, get in node', () => {
        napaZone.executeSync(NAPA_ZONE_TEST_MODULE, "storeSet", ['store1', 'd', { a: 1, b: 1}]);
        assert.deepEqual(store1.get('d'), {
            a: 1,
            b: 1
        });
    });

    it('transportable types: set in node, get in node', () => {
        store1.set('a', napa.memory.crtAllocator);
        assert.deepEqual(store1.get('a'), napa.memory.crtAllocator);
    });

    it('transportable types: set in node, get in napa', () => {
        store1.set('b', napa.memory.defaultAllocator);
        napaZone.executeSync(NAPA_ZONE_TEST_MODULE, "storeVerifyGet", ['store1', 'b', napa.memory.defaultAllocator]);
    });

    it('transportable types: set in napa, get in napa', () => {
        // We have to compare handle in this case, since napa.memory.defaultAllocator retrieved from napa zone will have 2+ refCount.
        napaZone.executeSync(NAPA_ZONE_TEST_MODULE, "storeSet", ['store1', 'e', napa.memory.defaultAllocator]);
        napaZone.executeSync(NAPA_ZONE_TEST_MODULE, "storeGetCompareHandle", ['store1', 'e', napa.memory.defaultAllocator.handle]);
    });

    it('transportable types: set in napa, get in node', () => {
        let debugAllocator = napa.memory.debugAllocator(napa.memory.defaultAllocator);
        napaZone.executeSync(NAPA_ZONE_TEST_MODULE, "storeSet", ['store1', 'f', debugAllocator]);
        assert.deepEqual(store1.get('f'), debugAllocator);
    });

    it('delete in node, check in node', () => {
        assert(store1.has('a'));
        store1.delete('a');
        assert(!store1.has('a'));
        assert(store1.get('a') === undefined);
        store1.delete('not-exist');
    });

    it('delete in node, check in napa', () => {
        assert(store1.has('b'));
        store1.delete('b');
        napaZone.executeSync(NAPA_ZONE_TEST_MODULE, "storeVerifyNotExist", ['store1', 'b']);
    });

    it('delete in napa, check in napa', () => {
        napaZone.executeSync(NAPA_ZONE_TEST_MODULE, "storeDelete", ['store1', 'c']);
        napaZone.executeSync(NAPA_ZONE_TEST_MODULE, "storeVerifyNotExist", ['store1', 'c']);
    })

    it('delete in napa, check in node', () => {
        napaZone.executeSync(NAPA_ZONE_TEST_MODULE, "storeDelete", ['store1', 'd']);
        assert(!store1.has('d'));
        assert(store1.get('d') === undefined);
    });

    it('size', () => {
        // set 'a', 'b', 'c', 'd', 'a', 'b', 'e', 'f'.
        // delete 'a', 'b', 'c', 'd'
        assert.equal(store1.size, 2);
    });

});
