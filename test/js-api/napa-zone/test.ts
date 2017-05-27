import * as napa from 'napajs';
import * as assert from 'assert';

export function bar(input: any) {
    return input;
}

export namespace ns1 {
    export namespace ns2 {
        export function foo(input: any) {
            return input;
        }
    }
}

export function getCurrentZone(): napa.zone.Zone {
    return napa.zone.current;
}

export function broadcast(id: string, code: string): void {
    let zone = napa.zone.get(id);
    // TODO: replace with broadcast when TODO:#3 is done.
    zone.broadcastSync(code);
}

export function broadcastTestFunction(id: string): void {
    // TODO: replace with broadcast when TODO:#3 is done.
    napa.zone.get(id).broadcastSync((input: string) => {
        console.log(input);
    }, ["hello world"]);
}

export function broadcastTransportable(id: string): void {
    // TODO: replace with broadcast when TODO:#3 is done.
    napa.zone.get(id).broadcastSync((input: any) => {
        console.log(input);
    }, [napa.memory.crtAllocator]);
}

export function broadcastClosure(id: string): void {
    let zone = napa.zone.get(id);
    // TODO: replace with broadcast when TODO:#3 is done.
    zone.broadcastSync(() => {
        console.log(zone);
    }, []);
}

export function execute(id: string, moduleName: string, functionName: string, args?: any[]): any {
    let zone = napa.zone.get(id);
    // TODO: replace with execute when TODO:#3 is done.
    return zone.executeSync(moduleName, functionName, args).value;
}

export function executeTestFunction(id: string): any {
    let zone = napa.zone.get(id);
    // TODO: replace with execute when TODO:#3 is done.
    return zone.executeSync((input: string) => {
            return input;
        }, ['hello world']);
}

/// <summary> Memory test helpers. </summary>
export function crtAllocatorTest(): void {
    let handle = napa.memory.crtAllocator.allocate(10);
    assert(!napa.memory.isEmpty(handle));
    napa.memory.crtAllocator.deallocate(handle, 10);
}

export function defaultAllocatorTest(): void {
    let handle = napa.memory.defaultAllocator.allocate(10);
    assert(!napa.memory.isEmpty(handle));
    napa.memory.defaultAllocator.deallocate(handle, 10);
}

export function debugAllocatorTest(): void {
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
}

let store2: napa.memory.store.Store = null;
export function getOrCreateStoreTest() {
    store2 = napa.memory.store.getOrCreate('store2');
    assert(store2 != null);
    assert.equal(store2.id, 'store2');
    assert.equal(store2.size, 0);
}

export function getStoreTest() {
    let store = napa.memory.store.get('store2');
    assert.equal(store.id, 'store2');

    // Store created from node zone.
    let store1 = napa.memory.store.get('store1');
    assert.equal(store1.id, 'store1');
}

export function storeVerifyGet(storeId: string, key: string, expectedValue: any) {
    let store = napa.memory.store.get(storeId);
    assert.deepEqual(store.get(key), expectedValue);
}

export function storeGetCompareHandle(storeId: string, key: string, expectedHandle: napa.memory.Handle) {
    let store = napa.memory.store.get(storeId);
    assert.deepEqual(store.get(key).handle, expectedHandle);
}

export function storeSet(storeId: string, key: string, expectedValue: any) {
    let store = napa.memory.store.get(storeId);
    store.set(key, expectedValue);
}

export function storeDelete(storeId: string, key: string) {
    let store = napa.memory.store.get(storeId);
    store.delete(key);
}

export function storeVerifyNotExist(storeId: string, key: string) {
    let store = napa.memory.store.get(storeId);
    assert(!store.has(key));
    assert(store.get(key) === undefined);
}

/// <summary> Transport test helpers. </summary>
export class CannotPass {
    field1: string;
    field2: number;
}

@napa.transport.cid(module.id)
export class CanPass extends napa.transport.TransportableObject {
    constructor(allocator: napa.memory.Allocator) {
        super();
        this._allocator = allocator;
    }
    
    save(payload: any, tc: napa.transport.TransportContext): void {
        payload['_allocator'] = this._allocator.marshall(tc) 
    }

    load(payload: any, tc: napa.transport.TransportContext): void {
        // Do nothing, as all members are transportable, which will be restored automatically.
    }

    _allocator: napa.memory.Allocator;
}

function testMarshallUnmarshall(input: any) {
    let tc = napa.transport.createTransportContext();
    let payload = napa.transport.marshall(input, tc);
    let expected = napa.transport.unmarshall(payload, tc);
    assert.deepEqual(input, expected);
}

export function simpleTypeTransportTest() {
    testMarshallUnmarshall({ 
        a: 'hello',
        b: {
            c: [0, 1]
        }
    });
}

export function jsTransportableTest() {
    testMarshallUnmarshall(new CanPass(napa.memory.crtAllocator));
}

export function addonTransportTest() {
    testMarshallUnmarshall(napa.memory.debugAllocator(napa.memory.crtAllocator));
}

export function compositeTransportTest() {
    testMarshallUnmarshall({
        a: napa.memory.debugAllocator(napa.memory.crtAllocator),
        b: [1, 2],
        c: {
            d: napa.memory.defaultAllocator,
            e: 1
        }
    });
}

export function nontransportableTest() {
    let tc = napa.transport.createTransportContext();
    let input = new CannotPass();
    let success = false;
    try {
        napa.transport.marshall(input, tc);
        success = true;
    }
    catch(error) {
    }
    assert(!success);
}