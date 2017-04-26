/// <summary> Describes the available settings for customizing a zone. </summary>
export interface ZoneSettings {

    /// <summary> The number of workers that will serve zone requests. </summary>
    workers?: number;

    /// <summary> A file that will be loaded on all zone workers in global scope. </summary>
    bootstrapFile?: string;
}

/// <summary> The type of the response code. </summary>
export type ResponseCode = number;

/// <summary> The type of the response value. </summary>
export type ResponseValue = string;

/// <summary> Describe zone available operations. </summary>
export interface Zone {
    
    /// <summary> The zone id. </summary>
    readonly id: string;

    /// <summary> Compiles and run the provided source code on all zone workers. </summary>
    /// <param name="source"> A valid javascript source code. </param>
    broadcast(source: string) : Promise<ResponseCode>;

    /// <summary> Executes the function on one of the zone workers. </summary>
    /// <param name="module"> The module that contains the function to execute. </param>
    /// <param name="func"> The function to execute. </param>
    /// <param name="args"> The arguments that will pass to the function. </param>
    /// <param name="timeout"> The timeout of the execution in milliseconds, defaults to infinity. </param>
    execute(module: string, func: string, args: any[], timeout?: number) : Promise<ResponseValue>;
}

