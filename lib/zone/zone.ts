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

/// <summary> Describe zone available operations. </summary>
export interface Zone {
    
    /// <summary> The zone id. </summary>
    readonly id: string;

    /// <summary> Compiles and run the provided source code on all zone workers. </summary>
    /// <param name="source"> A valid javascript source code. </param>
    broadcast(source: string) : Promise<void>;
    broadcastSync(source: string) : void;

    /// <summary> Compiles the function on all workers and runs it with the given arguments. </summary>
    /// <param name="func"> The JS function. </param>
    /// <param name="args"> The arguments that will pass to the function. </param>
    broadcast(func: Function, args: any[]) : Promise<void>;
    broadcastSync(func: Function, args: any[]) : void;

    /// <summary> Executes the function on one of the zone workers. </summary>
    /// <param name="module"> The module name that contains the function to execute. </param>
    /// <param name="func"> The function name to execute. </param>
    /// <param name="args"> The arguments that will pass to the function. </param>
    /// <param name="options"> Call options, defaults to DEFAULT_CALL_OPTIONS. </param>
    execute(module: string, func: string, args: any[], options?: CallOptions) : Promise<Result>;
    executeSync(module: string, func: string, args: any[], options?: CallOptions) : Result;

    /// <summary> Executes the function on one of the zone workers. </summary>
    /// <param name="func"> The JS function to execute. </param>
    /// <param name="args"> The arguments that will pass to the function. </param>
    /// <param name="options"> Call options, defaults to DEFAULT_CALL_OPTIONS. </param>
    execute(func: Function, args: any[], timeout?: number) : Promise<Result>;
    executeSync(func: Function, args: any[], timeout?: number) : Result;
}

