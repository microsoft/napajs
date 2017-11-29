// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

import * as path from 'path';
import * as zone from './zone';
import * as transport from '../transport';
import * as v8 from '../v8';

interface FunctionSpec {
    module: string;
    function: string;
    arguments: any[];
    options: zone.CallOptions;
    transportContext: transport.TransportContext;
}

class Result implements zone.Result{

     constructor(payload: string, transportContext: transport.TransportContext) {
          this._payload = payload;
          this._transportContext = transportContext; 
     }

     get value(): any {
         if (this._value == null) {
             this._value = transport.unmarshall(this._payload, this._transportContext);
         }

         return this._value;
     }

     get payload(): string {
         return this._payload; 
     }

     get transportContext(): transport.TransportContext {
         return this._transportContext; 
     }

     private _transportContext: transport.TransportContext;
     private _payload: string;
     private _value: any;
};

declare var __in_napa: boolean;

/// <summary> Helper function to workaround possible delay in Promise resolve/reject when working with Node event loop.
/// See https://github.com/audreyt/node-webworker-threads/issues/123#issuecomment-254019552
/// </summary>
function runImmediately(func : () => void) {
    if (typeof __in_napa === 'undefined') {
        // In node.
        setImmediate(func);
    } else  {
        // In napa workers.
        func();
    }
}

/// <summary> Zone consists of Napa isolates. </summary>
export class ZoneImpl implements zone.Zone {
    private _nativeZone: any;

    constructor(nativeZone: any) {
        this._nativeZone = nativeZone;
    }

    public get id(): string {
        return this._nativeZone.getId();
    }

    public toJSON(): any {
        return { id: this.id, type: this.id === 'node'? 'node': 'napa' };
    }

    public broadcast(arg1: any, arg2?: any) : Promise<void> {
        let source: string = this.createBroadcastSource(arg1, arg2);

        return new Promise<void>((resolve, reject) => {
            this._nativeZone.broadcast(source, (resultCode: number) => {
                runImmediately(() => {
                    if (resultCode === 0) {
                        resolve();
                    } else {
                        reject("broadcast failed with result code: " + resultCode);
                    }
                });
            });
        });
    }

    public execute(arg1: any, arg2?: any, arg3?: any, arg4?: any) : Promise<zone.Result> {
        let spec : FunctionSpec = this.createExecuteRequest(arg1, arg2, arg3, arg4);
        
        return new Promise<zone.Result>((resolve, reject) => {
            this._nativeZone.execute(spec, (result: any) => {
                runImmediately(() => {
                    if (result.code === 0) {
                        resolve(new Result(
                            result.returnValue,
                            transport.createTransportContext(true, result.contextHandle)));
                    } else {
                        reject(result.errorMessage);
                    }
                })
            });
        });
    }

    private createBroadcastSource(arg1: any, arg2?: any) : string {
        let source: string;
        if (typeof arg1 === "string") {
            // broadcast with source
            source = arg1;
        } else {
            // broadcast with function
            if (typeof arg1 != 'function') {
                throw new TypeError("Expected a Function type argument");
            }
            
            let functionString: string = (<Function>arg1).toString();
            
            // Prepare arguments as a string
            let argumentsString: string = "";
            if (arg2 != undefined) {
                if (!Array.isArray(arg2)) {
                    throw new TypeError("Expected an Array type argument");
                }
                argumentsString = JSON.stringify(arg2);
                argumentsString = argumentsString.substring(1, argumentsString.length - 1);
            }

            // Create a self invoking function string
            source = `(${ functionString })(${ argumentsString })`;
        }

        return source;
    }

    private createExecuteRequest(arg1: any, arg2: any, arg3?: any, arg4?: any) : FunctionSpec {

        let moduleName: string = null;
        let functionName: string = null;
        let args: any[] = null;
        let options: zone.CallOptions = undefined;

        if (typeof arg1 === 'function') {
            moduleName = "__function";
            if (arg1.origin == null) {
                // We get caller stack at index 2.
                // <caller> -> execute -> createExecuteRequest
                //   2           1               0
                arg1.origin = v8.currentStack(3)[2].getFileName();
            }

            functionName = transport.saveFunction(arg1);
            args = arg2;
            options = arg3;
        }
        else {
            moduleName = arg1;
            // If module name is relative path, try to deduce from call site.
            if (moduleName != null 
                && moduleName.length != 0 
                && !path.isAbsolute(moduleName)) {

                moduleName = path.resolve(
                    path.dirname(v8.currentStack(3)[2].getFileName()), 
                    moduleName);
            }
            functionName = arg2;
            args = arg3;
            options = arg4;
        }

        if (args == null) {
            args = [];
        }

        // Create a non-owning transport context which will be passed to execute call.
        let transportContext: transport.TransportContext = transport.createTransportContext(false);
        return {
            module: moduleName,
            function: functionName,
            arguments: (<Array<any>>args).map(arg => { return transport.marshall(arg, transportContext); }),
            options: options != null? options: zone.DEFAULT_CALL_OPTIONS,
            transportContext: transportContext
        };
    }
}