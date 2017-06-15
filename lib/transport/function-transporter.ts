////////////////////////////////////////////////////////////////////////
// Module to support function transport.

import { Store } from '../store/store';
import * as assert from 'assert';

/// <summary> Function hash to function cache. </summary>
let _hashToFunctionCache: {[hash: string]: (...args: any[]) => any} = {};

/// <summary> Function to hash cache. </summary>
/// <remarks> Function cannot be used as a key in TypeScript. </remarks>
let _functionToHashCache: any = {};

/// <summary> Marshalled function body cache. </summary>
let _store: Store;

/// <summary> Get underlying store to save marshall function body across isolates. </summary>
function getStore(): Store {
    if (_store == null) {
        // Lazy creation function store
        // 1) avoid circular runtime dependency between store and transport.
        // 2) avoid unnecessary cost when function transport is not used.
        _store = require('../store/store-api')
            .getOrCreate('__napajs_marshalled_functions');
    }
    return _store;
}

/// <summary> Save function and get a hash string to use it later. </summary>
export function save(func: (...args: any[]) => any): string {
    let hash = _functionToHashCache[(<any>(func))];
    if (hash == null) {
        // Should happen only on first marshall of input function in current isolate.
        let body = func.toString();
        hash = getFunctionHash(body);
        getStore().set(hash, body);
        cacheFunction(hash, func);
    }
    return hash;
}

/// <summary> Load a function with a hash retrieved from `save`. </summary>
export function load(hash: string): (...args: any[]) => any {
    let func = _hashToFunctionCache[hash];
    if (func == null) {
        // Should happen only on first unmarshall of given hash in current isolate..
        let body = getStore().get(hash);
        if (body == null) {
            throw new Error(`Function hash cannot be found: ${hash}`);
        }
        func = eval(`(${body})`);
        cacheFunction(hash, func);
    }
    return func;
}

/// <summary> Cache function with its hash in current isolate. </summary>
function cacheFunction(hash: string, func: (...args: any[]) => any) {
    _functionToHashCache[<any>(func)] = hash;
    _hashToFunctionCache[hash] = func;
}

/// <summary> Generate hash for function definition using DJB2 algorithm. 
/// See: https://en.wikipedia.org/wiki/DJB2 
/// </summary>
function getFunctionHash(functionDef: string): string {
    let hash = 5381;
    for (let i = 0; i < functionDef.length; ++i) {
        hash = (hash * 33) ^ functionDef.charCodeAt(i);
    }

    /* JavaScript does bitwise operations (like XOR, above) on 32-bit signed
    * integers. Since we want the results to be always positive, convert the
    * signed int to an unsigned by doing an unsigned bitshift. */
    return (hash >>> 0).toString(16);
}
