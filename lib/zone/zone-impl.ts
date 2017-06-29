import * as zone from './zone';
import * as transport from '../transport';

interface FunctionSpec {
    module: string;
    function: string;
    arguments: any[];
    options: zone.CallOptions;
    transportContext: transport.TransportContext;
}

class ExecuteResult implements zone.Result{

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
        let source: string = this.createBroadcastSource(arg1, arg2);

        return new Promise<void>((resolve, reject) => {
            this._nativeZone.broadcast(source, (resultCode: number) => {
                if (resultCode === 0) {
                    resolve();
                } else {
                    reject("broadcast failed with result code: " + resultCode);
                }
            });
        });
    }

    public execute(arg1: any, arg2: any, arg3?: any, arg4?: any) : Promise<zone.Result> {
        let spec : FunctionSpec = this.createExecuteRequest(arg1, arg2, arg3, arg4);
        
        return new Promise<zone.Result>((resolve, reject) => {
            this._nativeZone.execute(spec, (result: any) => {
                if (result.code === 0) {
                    resolve(new ExecuteResult(
                        result.returnValue,
                        transport.createTransportContext(result.contextHandle)));
                } else {
                    reject(result.errorMessage);
                }
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
            functionName = transport.saveFunction(arg1);
            args = arg2;
            options = arg3;
        }
        else {
            moduleName = arg1;
            functionName = arg2;
            args = arg3;
            options = arg4;
        }

        let transportContext: transport.TransportContext = transport.createTransportContext();
        return {
            module: moduleName,
            function: functionName,
            arguments: (<Array<any>>args).map(arg => { return transport.marshall(arg, transportContext); }),
            options: options != null? options: zone.DEFAULT_CALL_OPTIONS,
            transportContext: transportContext
        };
    }
}