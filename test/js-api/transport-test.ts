import * as napa from "napajs";
import * as assert from 'assert';

import * as t from './napa-zone/test';

describe('napajs/transport', () => {
    let napaZone = napa.zone.create('zone5');
    describe('TransportContext', () => {
        let tc = napa.transport.createTransportContext();
        let allocator = napa.memory.debugAllocator(napa.memory.crtAllocator);
        it('#saveShared', () => {
            tc.saveShared(allocator);
        });

        let shareable: napa.memory.Shareable = null;
        it('#loadShared', () => {
            shareable = tc.loadShared(allocator.handle);
            assert.deepEqual(shareable.handle, allocator.handle);
        });

        it('#sharedCount', () => {
            assert.equal(shareable.refCount, 3);
        });
    });

    describe('Transportable', () => {
        it('#IsTransportable', () => {
            assert(napa.transport.isTransportable(new t.CanPass(napa.memory.crtAllocator)));
            assert(napa.transport.isTransportable(1));
            assert(napa.transport.isTransportable('hello world'));
            assert(napa.transport.isTransportable([1, 2, 3]));
            assert(napa.transport.isTransportable([1, 2, new t.CanPass(napa.memory.crtAllocator)]));
            assert(napa.transport.isTransportable({ a: 1}));
            assert(napa.transport.isTransportable({ a: 1, b: new t.CanPass(napa.memory.crtAllocator)}));
            assert(!napa.transport.isTransportable(new t.CannotPass()));
            assert(!napa.transport.isTransportable([1, new t.CannotPass()]));
            assert(!napa.transport.isTransportable({ a: 1, b: new t.CannotPass()}));
        });
    });

    describe('Marshall/Unmarshall', () => {
        it('@node: simple types', () => {
            t.simpleTypeTransportTest();
        });
        
        it('@napa: simple types', () => {
            napaZone.executeSync('./napa-zone/test', "simpleTypeTransportTest", []);
        });

        it('@node: JS transportable', () => {
            t.jsTransportableTest();
        });

        it('@napa: JS transportable', () => {
            napaZone.executeSync('./napa-zone/test', "jsTransportableTest", []);
        });

        it('@node: addon transportable', () => {
           t.addonTransportTest();
        });

        it('@napa: addon transportable', () => {
            napaZone.executeSync('./napa-zone/test', "addonTransportTest", []);
        });

        it('@node: composite transportable', () => {
            t.compositeTransportTest();
        });

        it('@napa: composite transportable', () => {
            napaZone.executeSync('./napa-zone/test', "compositeTransportTest", []);
        });

        it('@node: non-transportable', () => {
            t.nontransportableTest();
        });

        it('@napa: non-transportable', () => {
            napaZone.executeSync('./napa-zone/test', "nontransportableTest", []);
        });
    });
});