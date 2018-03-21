// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

////////////////////////////////////////////////////////////////////////
// Module to support function transport.

import { Store } from '../store/store';
import * as assert from 'assert';
import * as path from 'path';

/// <summary> Function hash to function cache. </summary>
let _hashToFunctionCache: {[hash: string]: (...args: any[]) => any} = {};

/// <summary> Function to hash cache. </summary>
/// <remarks> Function cannot be used as a key in TypeScript. </remarks>
let _functionToHashCache: any = {};

/// <summary> Marshalled function body cache. </summary>
let _store: Store;

/// <summary> Interface for function definition that will be saved in store. </summary>
interface FunctionDef {
    /// <summary> From which file name the function is defined. </summary>
    origin: string;

    /// <summary> Function body. </summary>
    body: string;
}

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
        let origin = (<any>func).origin || '';
        let body = func.toString();
        let fullContent = origin + ":" + body;
        hash = getFunctionHash(fullContent);
        let def: FunctionDef = { 
            origin: origin,
            body: body
        };
        getStore().set(hash, def);
        cacheFunction(hash, func);
    }
    return hash;
}

/// <summary> Load a function with a hash retrieved from `save`. </summary>
export function load(hash: string): (...args: any[]) => any {
    let func = _hashToFunctionCache[hash];
    if (func == null) {
        // Should happen only on first unmarshall of given hash in current isolate..
        let def: FunctionDef = getStore().get(hash);
        if (def == null) {
            throw new Error(`Function hash cannot be found: ${hash}`);
        }
        func = loadFunction(def);
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
function getFunctionHash(signature: string): string {
    let hash = 5381;
    for (let i = 0; i < signature.length; ++i) {
        hash = (hash * 33) ^ signature.charCodeAt(i);
    }

    /* JavaScript does bitwise operations (like XOR, above) on 32-bit signed
    * integers. Since we want the results to be always positive, convert the
    * signed int to an unsigned by doing an unsigned bitshift. */
    return (hash >>> 0).toString(16);
}

/// <summary> Load function from definition. </summary>
function loadFunction(def: FunctionDef): (...args: any[]) => any {
    let moduleId = def.origin;
    let script = "module.exports = " + def.body + ";";
    let func: any = null;

    let Module: any = null;
    if (Module == null) {
        Module = require('module');
    }
    let module = new Module(moduleId);
    module.filename = moduleId;
    module.paths = Module._nodeModulePaths(path.dirname(def.origin));
    module._compile(script, moduleId);
    func = module.exports;

    func.origin = def.origin;
    return func;
}
