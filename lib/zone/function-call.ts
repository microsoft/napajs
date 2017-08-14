// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

import * as transport from '../transport';
import { CallOptions } from './zone';

/// <summary> Rejection type </summary>
/// TODO: we need a better mapping between error code and result code.
export enum RejectionType {
    TIMEOUT = 3,
    APP_ERROR = 6
}

/// <summary> Interface for Call context. </summary>
export interface CallContext {

    /// <summary> Resolve task with marshalled result. </summary>
    resolve(result: string): void;

    /// <summary> Reject task with reason. </summary>
    reject(reason: any): void;

    /// <summary> Reject task with a rejection type and reason. </summary>
    reject(type: RejectionType, reason: any): void;

    /// <summary> Returns whether task has finished (either completed or cancelled). </summary>
    readonly finished: boolean;

    /// <summary> Elapse in nano-seconds since task started. </summary>
    readonly elapse: [number, number];

    /// <summary> Module name to select function. </summary>
    readonly module: string;

    /// <summary> Function name to execute. </summary>
    readonly function: string;

    /// <summary> Marshalled arguments. </summary>
    readonly args: string[];

    /// <summary> Transport context. </summary>
    readonly transportContext: transport.TransportContext;

    /// <summary> Execute options. </summary>
    readonly options: CallOptions;
}

/// <summary> 
///     Proxy function for __napa_zone_call__. 
///     1) calling a global function: 
///        module name: undefined or empty string
///        function name: global function name
///     2) calling an anonymous function at client side: 
///        module name: literal '__function' 
///        function name: hash returned from transport.saveFunction().
///     3) calling a function from a module:
///        module name: target module path.
///        function name: target function name from the module.
///
///     function name can have multiple levels like 'foo.bar'.
/// </summary>
export function call(context: CallContext): void {
    // Cache the context since every call to context.transportContext will create a new wrap upon inner TransportContext pointer.
    let transportContext = context.transportContext;
    let result: any = undefined;
    try {
        result = callFunction(
            context.module, 
            context.function, 
            context.args, 
            transportContext,
            context.options);
    }
    catch(error) {
        context.reject(error);
        return;
    }

    if (result != null 
        && typeof result === 'object'
        && typeof result['then'] === 'function') {
        // Delay completion if return value is a promise.
        result.then((value: any) => {
            finishCall(context, transportContext, value);
        })
        .catch((error: any) => {
            context.reject(error);
        });
        return;
    }
    finishCall(context, transportContext, result);
}

/// <summary> Call a function. </summary>
function callFunction(
    moduleName: string, 
    functionName: string, 
    marshalledArgs: string[], 
    transportContext: transport.TransportContext,
    options: CallOptions): any {

    let module: any = null;
    let useAnonymousFunction: boolean = false;

    if (moduleName == null || moduleName.length === 0 || moduleName === 'global') {
        module = global;
    } else if (moduleName === '__function') {
        useAnonymousFunction = true;
    } else {
        module = require(moduleName);
    }

    let func = null;
    if (!useAnonymousFunction) {
        if (module == null) {
            throw new Error(`Cannot load module \"${moduleName}\".`);
        }
        func = module;
        if (functionName != null && functionName.length != 0) {
            var path = functionName.split('.');
            for (let item of path) {
                func = func[item];
                if (func === undefined) {
                    throw new Error("Cannot find function '" + functionName + "' in module '" + moduleName + "'");
                }
            }
        }
        if (typeof func !== 'function') {
            throw new Error("'" + functionName + "' in module '" + moduleName + "' is not a function");
        }
    } else {
        // Anonymous function.
        func = transport.loadFunction(functionName);
    }

    let args = marshalledArgs.map((arg) => { return transport.unmarshall(arg, transportContext); });
    return func.apply(this, args);
}

/// <summary> Finish call with result. </summary>
function finishCall(
    context: CallContext, 
    transportContext: transport.TransportContext, 
    result: any) {

    let payload: string = undefined;
    try {
        payload = transport.marshall(result, transportContext);
    }
    catch (error) {
        context.reject(error);
        return;
    }
    context.resolve(payload);
}