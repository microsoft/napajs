import * as addon from '../bin/addon';

declare var __in_napa: boolean;

/// <summary> Describes the available settings for customizing a zone. </summary>
export interface ZoneSettings {

    /// <summary> The number of workers that will serve zone requests. </summary>
    workers?: number;

    /// <summary> A file that will be loaded on all zone workers in global scope. </summary>
    bootstrapFile?: string;
}

/// <summary> 
///     Describes the available platform settings, these setting include cross zone settings
///     as well as zone specific settings. The zone specific settings are used as defaults
///     when a zone is created without specifying them.
/// </summary>
export interface PlatformSettings extends ZoneSettings {

    /// <summary> The logging provider to use when outputting logs. </summary>
    loggingProvider?: string;

    /// <summary> The metric provider to use when creating/setting metric values. </summary>
    metricProvider?: string;
}

/// <summary> The type of the response code. </summary>
type ResponseCode = number;

/// <summary> The type of the response value. </summary>
type ResponseValue = string;

/// <summary> Describe zone available operations. </summary>
export interface Zone {
    
    /// <summary> The zone id. </summary>
    readonly id: string;

    /// <summary> Loads the source code into the global scope in all zone workers. </summary>
    /// <param name="source"> A valid javascript source code. </param>
    load(source: string) : Promise<ResponseCode>;

    /// <summary> Executes the function on one of the zone workers. </summary>
    /// <param name="module"> The module that contains the function to execute. </param>
    /// <param name="func"> The function to execute. </param>
    /// <param name="args"> The arguments that will pass to the function. </param>
    /// <param name="timeout"> The timeout of the execution in milliseconds, defaults to infinity. </param>
    execute(module: string, func: string, args: any[], timeout?: number) : Promise<ResponseValue>;

    /// <summary> Executes the function on all zone workers. </summary>
    /// <param name="module"> The module that contains the function to execute. </param>
    /// <param name="func"> The function to execute. </param>
    /// <param name="args"> The arguments that will pass to the function. </param>
    executeAll(module: string, func: string, args: any[]) : Promise<ResponseCode>;

    /// <summary> Destroys the zone, cleaning all resources associated with it. </summary>
    destory(): void;
}


/// <summary> Initialization of napa is only needed if we run in node. </summary>
let _initializationNeeded: boolean = (typeof __in_napa === undefined);

/// <summary> Empty platform settings. </summary>
let _platformSettings: PlatformSettings = {};

/// <summary> Returns the logging provider. </summary>
export function getLoggingProvider() {
    return addon.getLoggingProvider();
}

/// <summary> Returns the metric provider. </summary>
export function getMetricProvider() {
    return addon.getMetricProvider();
}

/// <summary> Sets the platform settings. Must be called fron node before the first container is created. </summary>
export function setPlatformSettings(settings: PlatformSettings) {
    if (!_initializationNeeded) {
        // If we don't need to initialize we can't set platform settings.
        throw new Error("Cannot set platform settings after napa was already initialized");
    }

    _platformSettings = settings;
}

/// <summary> Creates a new zone. </summary>
/// <summary> A unique id to identify the zone. </summary>
/// <param name="settings"> The settings of the new zone. </param>
export function createZone(id: string, settings?: ZoneSettings) : Zone {
    if (_initializationNeeded) {
        // Lazy initialization of napa when first zone is created.
        addon.initialize(_platformSettings);
        _initializationNeeded = false;
    }

    return new ZoneWrapper(addon.createZone(id, settings));
}

/// <summary> Returns the zone associated with the provided id. </summary>
export function getZone(id: string) : Zone {
    if (id === "node") {
        return new NodeZone();
    }

    return new ZoneWrapper(addon.getZone(id));
}

/// <summary> Returns the current zone. </summary>
export let currentZone: Zone = getCurrentZone();

function getCurrentZone() : Zone {
    if (typeof __in_napa === undefined) {
        return new ZoneWrapper(addon.getCurrentZone());
    }
    
    return new NodeZone();
}

class ZoneWrapper implements Zone {
    private _nativeZone;

    constructor(nativeZone) {
        this._nativeZone = nativeZone;
    }

    public get id(): string {
        return this._nativeZone.getId();
    }

    public load(source: string) : Promise<ResponseCode> {
        if (typeof __in_napa !== undefined) {
            // Napa does not support async yet, we wrap this sync call in a promise
            // to provide a uniform API.
            return Promise.resolve(this._nativeZone.loadSync(source));
        }

        return new Promise<ResponseCode>(resolve => {
            this._nativeZone.load(source, resolve);
        });
    }

    public execute(module: string, func: string, args: any[], timeout?: number) : Promise<ResponseValue> {
        // Convert all arguments to strings
        args = args.map(x => { return JSON.stringify(x); });

        if (typeof __in_napa !== undefined) {
            // Napa does not support async yet, we wrap this sync call in a promise
            // to provide a uniform API.
            return Promise.resolve(this._nativeZone.executeSync(module, func, args, timeout));
        }

        return new Promise<ResponseValue>((resolve, reject) => {
            this._nativeZone.execute(module, func, args, (response) => {
                if (response.code === 0) {
                    resolve(response.returnValue);
                } else {
                    reject(response.errorMessage);
                }
            }, timeout);
        });
    }

    public executeAll(module: string, func: string, args: any[]) : Promise<ResponseCode> {
        // Convert all arguments to strings
        args = args.map(x => { return JSON.stringify(x); });
        
        if (typeof __in_napa !== undefined) {
            // Napa does not support async yet, we wrap this sync call in a promise
            // to provide a uniform API.
            return Promise.resolve(this._nativeZone.executeAllSync(module, func, args));
        }

        return new Promise<ResponseCode>(resolve => {
            this._nativeZone.executeAll(module, func, args, resolve);
        });
    }

    public destory(): void {
        this._nativeZone.destroy();
    }
}


class NodeZone implements Zone {
    
    public get id(): string {
        return "node";
    }

    public load(source: string) : Promise<ResponseCode> {
        // TODO @asib: add implementation
        return undefined;
    }

    public execute(module: string, func: string, args: string[], timeout?: number) : Promise<ResponseValue> {
        // TODO @asib: add implementation
        return undefined;
    }

    public executeAll(module: string, func: string, args: string[]) : Promise<ResponseCode> {
        // TODO @asib: add implementation
        return undefined;
    }

    public destory(): void {
        // TODO @asib: add implementation
    }
}
