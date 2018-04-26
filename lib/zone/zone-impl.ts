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
        let spec: FunctionSpec = this.createBroadcastRequest(arg1, arg2);

        return new Promise<void>((resolve, reject) => {
            this._nativeZone.broadcast(spec, (result: any) => {
                setImmediate(() => {
                    if (result.code === 0) {
                        resolve();
                    } else {
                        reject(result.errorMessage);
                    }
                });
            });
        });
    }

    public broadcastSync(arg1: any, arg2?: any) : void {
        let spec: FunctionSpec = this.createBroadcastRequest(arg1, arg2);
        let result = this._nativeZone.broadcastSync(spec);
        if (result.code !== 0) {
            throw new Error(result.errorMessage);
        }
    }

    public execute(arg1: any, arg2?: any, arg3?: any, arg4?: any) : Promise<zone.Result> {
        let spec : FunctionSpec = this.createExecuteRequest(arg1, arg2, arg3, arg4);
        
        return new Promise<zone.Result>((resolve, reject) => {
            this._nativeZone.execute(spec, (result: any) => {
                setImmediate(() => {
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

    public recycle(): void {
        this._nativeZone.recycle();
    }

    private createBroadcastRequest(arg1: any, arg2?: any) : FunctionSpec {
        if (typeof arg1 === "function") {
            // broadcast with function
            if (arg1.origin == null) {
                // We get caller stack at index 2.
                // <caller> -> broadcast -> createBroadcastRequest
                //   2           1               0
                arg1.origin = v8.currentStack(3)[2].getFileName();
            }
            return {
                module: "__function",
                function: transport.saveFunction(arg1),
                arguments: (arg2 == null
                           ? []
                           : (<Array<any>>arg2).map(arg => transport.marshall(arg, null))),
                options: zone.DEFAULT_CALL_OPTIONS,
                transportContext: null
            };
        } else {
            // broadcast with source
            return {
                module: "",
                function: "eval",
                arguments: [JSON.stringify(arg1)],
                options: zone.DEFAULT_CALL_OPTIONS,
                transportContext: null
            };
        }
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
            arguments: (<Array<any>>args).map(arg => transport.marshall(arg, transportContext)),
            options: options != null? options: zone.DEFAULT_CALL_OPTIONS,
            transportContext: transportContext
        };
    }
}