// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

import * as assert from 'assert';
import * as path from "path";
import * as napa from '../../lib/index';

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

export function broadcast(id: string, code: string): Promise<void> {
    let zone = napa.zone.get(id);
    return zone.broadcast(code);
}

export function broadcastTestFunction(id: string): Promise<void> {
    return napa.zone.get(id).broadcast((input: string) => {
            console.log(input);
        }, ["hello world"]);
}

export function broadcastTransportable(id: string): Promise<void> {
    return napa.zone.get(id).broadcast((input: any) => {
            console.log(input);
        }, [napa.memory.crtAllocator]);
}

export function broadcastClosure(id: string): Promise<void> {
    let zone = napa.zone.get(id);
    return zone.broadcast(() => {
            console.log(zone);
        }, []);
}

export function execute(id: string, moduleName: string, functionName: string, args?: any[]): Promise<any> {
    let zone = napa.zone.get(id);
    return new Promise((resolve, reject) => {
        zone.execute(moduleName, functionName, args)
            .then((result: napa.zone.Result) => resolve(result.value))
            .catch((error: any) => reject(error));
    });
}

export function executeTestFunction(id: string): Promise<any> {
    let zone = napa.zone.get(id);
    return new Promise((resolve, reject) => {
        zone.execute((input: string) => { return input; }, ['hello world'])
            .then((result: napa.zone.Result) => resolve(result.value))
            .catch((error: any) => reject(error));
    });
}

export function executeTestFunctionWithClosure(id: string): Promise<any> {
    let zone = napa.zone.get(id);
    return new Promise((resolve, reject) => {
        zone.execute(() => { return zone; }, [])
            .then((result: napa.zone.Result) => resolve(result.value))
            .catch((error: any) => reject(error));
    });
}

export function waitMS(waitTimeInMS: number): number {
    var start = new Date().getTime();
    var wait = 0;
    do {
        wait = new Date().getTime() - start;
    } while (wait < waitTimeInMS);
    return wait - waitTimeInMS;
}

export function executeTestFunctionWithTimeout(id: string, waitTimeInMS: number, timeoutInMS?: number): Promise<any> {
    timeoutInMS = timeoutInMS ? timeoutInMS : Number.MAX_SAFE_INTEGER;
    let zone = napa.zone.get(id);
    return new Promise((resolve, reject) => {
        zone.execute(waitMS, [waitTimeInMS], {timeout: timeoutInMS})
            .then((result: napa.zone.Result) => resolve(result.value))
            .catch((error: any) => reject(error));
    });
}

export function executeWithTransportableArgs(id: string): Promise<any> {
    let zone = napa.zone.get(id);
    return new Promise((resolve, reject) => {
        zone.execute((allocator: napa.memory.Allocator) => {
                var assert = require("assert");
                assert.deepEqual(allocator.handle, (<any>global).napa.memory.crtAllocator.handle);
                return 1;
            }, [napa.memory.crtAllocator])
            .then ((result: napa.zone.Result) => resolve(result.value))
            .catch((error: any) => reject(error));
    });
}

export function executeWithTransportableReturns(id: string): Promise<any> {
    let zone = napa.zone.get(id);
    return new Promise((resolve, reject) => {
        zone.execute((allocator: napa.memory.Allocator) => {
                return allocator;
            }, [napa.memory.crtAllocator])
            .then ((result: napa.zone.Result) => resolve(result.value))
            .catch((error: any) => reject(error));
    });
}

export function executeWithArgsContainingUnicodeString(id: string): Promise<any> {
    let unicodeStr = "中文 español deutsch English हिन्दी العربية português বাংলা русский 日本語 ਪੰਜਾਬੀ 한국어 தமிழ் עברית";
    let zone = napa.zone.get(id);
    return new Promise((resolve, reject) => {
        zone.execute((str: string) => {
                var assert = require("assert");
                let unicodeStr = "中文 español deutsch English हिन्दी العربية português বাংলা русский 日本語 ਪੰਜਾਬੀ 한국어 தமிழ் עברית";
                assert.equal(str, unicodeStr);
                return str;
            }, [unicodeStr])
            .then ((result: napa.zone.Result) => resolve(result.value))
            .catch((error: any) => reject(error));
    });
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

let store2: napa.store.Store = null;
export function getOrCreateStoreTest() {
    store2 = napa.store.getOrCreate('store2');
    assert(store2 != null);
    assert.equal(store2.id, 'store2');
    assert.equal(store2.size, 0);
}

export function getStoreTest() {
    let store = napa.store.get('store2');
    assert.equal(store.id, 'store2');

    // Store created from node zone.
    let store1 = napa.store.get('store1');
    assert.equal(store1.id, 'store1');
}

export function storeVerifyGet(storeId: string, key: string, expectedValue: any) {
    let store = napa.store.get(storeId);
    assert.deepEqual(store.get(key), expectedValue);
}

export function storeGetCompareHandle(storeId: string, key: string, expectedHandle: napa.memory.Handle) {
    let store = napa.store.get(storeId);
    assert.deepEqual(store.get(key).handle, expectedHandle);
}

export function storeSet(storeId: string, key: string, expectedValue: any) {
    let store = napa.store.get(storeId);
    store.set(key, expectedValue);
}

export function storeDelete(storeId: string, key: string) {
    let store = napa.store.get(storeId);
    store.delete(key);
}

export function storeVerifyNotExist(storeId: string, key: string) {
    let store = napa.store.get(storeId);
    assert(!store.has(key));
    assert(store.get(key) === undefined);
}

/// <summary> Transport test helpers. </summary>
export class CannotPass {
    field1: string;
    field2: number;
}

@napa.transport.cid()
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
    assert.equal(input.toString(), expected.toString());
}

export function simpleTypeTransportTest() {
    testMarshallUnmarshall({ 
        a: 'hello',
        b: {
            c: [0, 1]
        }
    });
}

export function jsTransportTest() {
    testMarshallUnmarshall(new CanPass(napa.memory.crtAllocator));
}

export function functionTransportTest() {
    testMarshallUnmarshall(() => { return 0; });
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