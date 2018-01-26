// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

import * as transport from "../transport";

/// <summary> Describes the available settings for customizing a zone. </summary>
export interface ZoneSettings {

    /// <summary> The number of workers that will serve zone requests. </summary>
    workers?: number;
}

/// <summary> Default ZoneSettings </summary>
export let DEFAULT_SETTINGS: ZoneSettings = {

    /// <summary> Set default workers to 2. </summary>
    workers: 2
};

/// <summary> Represent option to transport arguments in zone.execute. </summary>
export enum TransportOption {

    /// <summary> transport.marshall/unmarshall will be done by `napajs` automatically.
    /// This is the most common way, but may not be performance optimal with objects
    /// that will be shared in multiple zone.execute.
    /// </summary>
    AUTO,

    /// <summary> transport.marshall/unmarshall will be done by user manually. </summary>
    MANUAL,
}

/// <summary> Represent the options of calling a function. </summary>
export interface CallOptions {

    /// <summary> Timeout in milliseconds. By default set to 0 if timeout is not needed. </summary>
    timeout?: number,

    /// <summary> Transport option on passing arguments. By default set to TransportOption.AUTO </summary>
    transport?: TransportOption
}

/// <summary> Default execution options. </summary>
export let DEFAULT_CALL_OPTIONS: CallOptions = {

    /// <summary> No timeout. </summary>
    timeout: 0,

    /// <summary> Set argument transport option to automatic. </summary>
    transport: TransportOption.AUTO
}

/// <summary> Represent the result of an execute call. </summary>
export interface Result {

    /// <summary> The unmarshalled result value. </summary>
    readonly value : any;

    /// <summary> A marshalled result. </summary>
    readonly payload : string;

    /// <summary> Transport context carries additional information needed to unmarshall. </summary>
    readonly transportContext : transport.TransportContext;
}

/// <summary>
///     Interface for Zone (for both Napa zone and Node zone)
///     A `zone` consists of one or multiple JavaScript threads, we name each thread `worker`.
///     Workers within a zone are symmetric, which means execute on any worker from the zone should return the same result,
///     and the internal state of every worker should be the same from long lasting point of view.
///
///     There are 2 operations, designed to reinforce the symmetry of workers:
///     1) Broadcast - run code that changes worker state on all workers, returning a promise for pending operation. 
///        Through the promise, we can only know if operation succeed or failed. Usually we use `broadcast` to bootstrap
///        application, pre-cache objects, or change application settings.
///     2) Execute - run code that doesn't change worker state on an arbitrary worker, returning a promise of getting the result.
///        Execute is designed for doing the real work.
/// 
///     Zone operations are on a basis of first-come-first-serve, while `broadcast` takes higher priority over `execute`.
/// </summary>
export interface Zone {
    
    /// <summary> The zone id. </summary>
    readonly id: string;

    /// <summary> Compiles and run the provided source code on all zone workers. </summary>
    /// <param name="source"> A valid javascript source code. </param>
    /// <returns> A promise which is resolved when broadcast completes, and rejected when failed. </returns>
    /// <remarks>
    ///     Broadcast is designed for the purpose of bootstrapping/changing internal state on all workers.
    ///     Function broadcast returns a promise of void, telling whether the operation succeeded or failed.
    ///     Promise will be rejected on failure from any worker, though most likely all workers will fail if one fails.
    /// </remarks>
    broadcast(source: string) : Promise<void>;

    /// <summary> Run the function on all workers with the given arguments. </summary>
    /// <param name="func"> The JS function. </param>
    /// <param name="args"> The arguments that will pass to the function. </param>
    /// <returns> A promise which is resolved when broadcast completes and rejected when failed. </returns>
    /// <remarks>
    ///     Broadcast is designed for the purpose of bootstrapping/changing internal state on all workers.
    ///     Function broadcast returns a promise of void, telling whether the operation succeeded or failed.
    ///     Promise will be rejected on failure from any worker, though most likely all workers will fail if one fails.
    /// </remarks>
    broadcast(func: (...args: any[]) => void, args?: any[]) : Promise<void>;

    /// <summary> Run the function on all workers with the given arguments. </summary>
    /// <param name="func"> The JS function. </param>
    /// <param name="args"> The arguments that will pass to the function. </param>
    /// <returns> A promise which is resolved when broadcast completes and rejected when failed. </returns>
    /// <remarks>
    ///     Broadcast is designed for the purpose of bootstrapping/changing internal state on all workers.
    ///     Function broadcast returns a promise of void, telling whether the operation succeeded or failed.
    ///     Promise will be rejected on failure from any worker, though most likely all workers will fail if one fails.
    /// </remarks>
    broadcast(func: (...args: any[]) => Promise<void>, args?: any[]) : Promise<void>;

    /// <summary> Compiles and run the provided source code on all zone workers synchronously. </summary>
    /// <param name="source"> A valid javascript source code. </param>
    /// <remarks>
    ///     Broadcast is designed for the purpose of bootstrapping/changing internal state on all workers.
    ///     Function broadcastSync wait for all workers to complete.
    ///     An exception will be thrown when any worker fails.
    /// </remarks>
    broadcastSync(source: string) : void;

    /// <summary> Run the function on all workers with the given arguments. </summary>
    /// <param name="func"> The JS function. </param>
    /// <param name="args"> The arguments that will pass to the function. </param>
    /// <returns> A promise which is resolved when broadcast completes and rejected when failed. </returns>
    /// <remarks>
    ///     Broadcast is designed for the purpose of bootstrapping/changing internal state on all workers.
    ///     Function broadcast returns a promise of void, telling whether the operation succeeded or failed.
    ///     Promise will be rejected on failure from any worker, though most likely all workers will fail if one fails.
    /// </remarks>
    broadcastSync(func: (...args: any[]) => void, args?: any[]) : void;

    /// <summary> Run the function on all workers with the given arguments. </summary>
    /// <param name="func"> The JS function. </param>
    /// <param name="args"> The arguments that will pass to the function. </param>
    /// <returns> A promise which is resolved when broadcast completes and rejected when failed. </returns>
    /// <remarks>
    ///     Broadcast is designed for the purpose of bootstrapping/changing internal state on all workers.
    ///     Function broadcast returns a promise of void, telling whether the operation succeeded or failed.
    ///     Promise will be rejected on failure from any worker, though most likely all workers will fail if one fails.
    /// </remarks>
    broadcastSync(func: (...args: any[]) => Promise<void>, args?: any[]) : void;

    /// <summary> Executes the function on one of the zone workers. </summary>
    /// <param name="module"> The module name that contains the function to execute. </param>
    /// <param name="func"> The function name to execute. </param>
    /// <param name="args"> The arguments that will pass to the function. </param>
    /// <param name="options"> Call options, defaults to DEFAULT_CALL_OPTIONS. </param>
    /// <returns> A promise of result which is resolved when execute completes, and rejected when failed. </returns>
    execute(module: string, func: string, args?: any[], options?: CallOptions) : Promise<Result>;

    /// <summary> Executes the function on one of the zone workers. </summary>
    /// <param name="func"> The JS function to execute. </param>
    /// <param name="args"> The arguments that will pass to the function. </param>
    /// <param name="options"> Call options, defaults to DEFAULT_CALL_OPTIONS. </param>
    /// <returns> A promise of result which is resolved when execute completes, and rejected when failed. </returns>
    execute(func: (...args: any[]) => any, args?: any[], options?: CallOptions) : Promise<Result>;
}

