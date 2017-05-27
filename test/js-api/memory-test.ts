import * as napa from "napajs";
import * as assert from 'assert';

describe('napajs/memory', () => {
    let napaZone = napa.zone.create('zone5');

    describe('Handle', () => {
        it('#isEmpty', () => {
            assert(napa.memory.isEmpty([0, 0]));
            assert(!napa.memory.isEmpty([1, 2]));
        });
    });

    describe('Shareable', () => {
        it('#isShareable', () => {
            assert(napa.memory.isShareable(napa.memory.crtAllocator));
            assert(!napa.memory.isShareable("hello world"));
            assert(!napa.memory.isShareable(1));
            assert(!napa.memory.isShareable({ a: 1 }));
        });
    });

    describe('Allocators', () => {
        it('@node: crtAllocator', () => {
            let handle = napa.memory.crtAllocator.allocate(10);
            assert(!napa.memory.isEmpty(handle));
            napa.memory.crtAllocator.deallocate(handle, 10);
        });
        
        it('@napa: crtAllocator', () => {
            napaZone.executeSync('./napa-zone/test', "crtAllocatorTest", []);
        });
    
        it('@node: defaultAllocator', () => {
            let handle = napa.memory.defaultAllocator.allocate(10);
            assert(!napa.memory.isEmpty(handle));
            napa.memory.defaultAllocator.deallocate(handle, 10);
        });

        it('@napa: defaultAllocator', () => {
            napaZone.executeSync('./napa-zone/test', "defaultAllocatorTest", []);
        });

        it('@node: debugAllocator', () => {
            let allocator = napa.memory.debugAllocator(napa.memory.defaultAllocator);
            let handle = allocator.allocate(10);
            assert(!napa.memory.isEmpty(handle));
            allocator.deallocate(handle, 10);
            let debugInfo = JSON.parse(allocator.getDebugInfo());
            assert.deepEqual(debugInfo, {
                allocate: 1,
                allocatedSize: 10,
                deallocate: 1,
                deallocatedSize: 10
            });
        });

        it('@napa: debugAllocator', () => {
            napaZone.executeSync('./napa-zone/test', "debugAllocatorTest", []);
        });
    });

    describe('Store', () => {
        let store1 = napa.memory.store.create('store1');
        it('@node: store.create - succeed', () => {
            assert(store1 != null);
            assert.equal(store1.id, 'store1');
            assert.equal(store1.size, 0);
        });

        it('@node: store.create - already exists', () => {
            let succeed = false;
            try {
                let store = napa.memory.store.create('store1');
                succeed = true;
            }
            catch (error) {
            }
            assert(!succeed);
        });

        it('@napa: store.getOrCreate', () => {
            napaZone.executeSync('./napa-zone/test', "getOrCreateStoreTest", []);
        });

        it('@node: store.get', () => {
            let store = napa.memory.store.get('store1');
            assert.equal(store.id, store1.id);

            // Store created from napa zone.
            let store2 = napa.memory.store.get('store2');
            assert.equal(store2.id, 'store2');
        });

        it('@napa: store.get', () => {
            napaZone.executeSync('./napa-zone/test', "getStoreTest", []);
        });

        it('simple types: set in node, get in node', () => {
            store1.set('a', 1);
            assert.equal(store1.get('a'), 1);
        });

        it('simple types: set in node, get in napa', () => {
            store1.set('b', 'hi');
            napaZone.executeSync('./napa-zone/test', "storeVerifyGet", ['store1', 'b', 'hi']);
        });

        it('simple types: set in napa, get in napa', () => {
            napaZone.executeSync('./napa-zone/test', "storeSet", ['store1', 'c', 1]);
            napaZone.executeSync('./napa-zone/test', "storeVerifyGet", ['store1', 'c', 1]);
        });

        it('simple types: set in napa, get in node', () => {
            napaZone.executeSync('./napa-zone/test', "storeSet", ['store1', 'd', { a: 1, b: 1}]);
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
            napaZone.executeSync('./napa-zone/test', "storeVerifyGet", ['store1', 'b', napa.memory.defaultAllocator]);
        });

        it('transportable types: set in napa, get in napa', () => {
            // We have to compare handle in this case, since napa.memory.defaultAllocator retrieved from napa zone will have 2+ refCount.
            napaZone.executeSync('./napa-zone/test', "storeSet", ['store1', 'e', napa.memory.defaultAllocator]);
            napaZone.executeSync('./napa-zone/test', "storeGetCompareHandle", ['store1', 'e', napa.memory.defaultAllocator.handle]);
        });

        it('transportable types: set in napa, get in node', () => {
            let debugAllocator = napa.memory.debugAllocator(napa.memory.defaultAllocator);
            napaZone.executeSync('./napa-zone/test', "storeSet", ['store1', 'f', debugAllocator]);
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
            napaZone.executeSync('./napa-zone/test', "storeVerifyNotExist", ['store1', 'b']);
        });

        it('delete in napa, check in napa', () => {
            napaZone.executeSync('./napa-zone/test', "storeDelete", ['store1', 'c']);
            napaZone.executeSync('./napa-zone/test', "storeVerifyNotExist", ['store1', 'c']);
        })

        it('delete in napa, check in node', () => {
            napaZone.executeSync('./napa-zone/test', "storeDelete", ['store1', 'd']);
            assert(!store1.has('d'));
            assert(store1.get('d') === undefined);
        });

        it('size', () => {
            // set 'a', 'b', 'c', 'd', 'a', 'b', 'e', 'f'.
            // delete 'a', 'b', 'c', 'd'
            assert.equal(store1.size, 2);
        });
    });
});