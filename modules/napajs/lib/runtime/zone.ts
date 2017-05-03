import * as transport from "../transport";

/// <summary> Describes the available settings for customizing a zone. </summary>
export interface ZoneSettings {

    /// <summary> The number of workers that will serve zone requests. </summary>
    workers?: number;

    /// <summary> A file that will be loaded on all zone workers in global scope. </summary>
    bootstrapFile?: string;
}

/// <summary> The type of the response code. </summary>
export type ResponseCode = number;

/// <summary> Represent the result of an execute call. </summary>
export interface ExecuteResult {

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
    broadcast(source: string) : Promise<ResponseCode>;

    /// <summary> Compiles the function on all workers and runs it with the given arguments. </summary>
    /// <param name="func"> The JS function. </param>
    /// <param name="args"> The arguments that will pass to the function. </param>
    broadcast(func: Function, args: any[]) : Promise<ResponseCode>;

    /// <summary> Executes the function on one of the zone workers. </summary>
    /// <param name="module"> The module name that contains the function to execute. </param>
    /// <param name="func"> The function name to execute. </param>
    /// <param name="args"> The arguments that will pass to the function. </param>
    /// <param name="timeout"> The timeout of the execution in milliseconds, defaults to infinity. </param>
    execute(module: string, func: string, args: any[], timeout?: number) : Promise<ExecuteResult>;

    /// <summary> Executes the function on one of the zone workers. </summary>
    /// <param name="func"> The JS function to execute. </param>
    /// <param name="args"> The arguments that will pass to the function. </param>
    /// <param name="timeout"> The timeout of the execution in milliseconds, defaults to infinity. </param>
    execute(func: Function, args: any[], timeout?: number) : Promise<ExecuteResult>;
}

