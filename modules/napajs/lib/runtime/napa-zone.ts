import * as zone from "./zone";
import * as transport from "../transport";

declare var __in_napa: boolean;

interface ExecuteRequest {
    module: string;
    function: string;
    arguments: any[];
    timeout: number;
    transportContext: transport.TransportContext;
}

export class NapaExecuteResult implements zone.ExecuteResult{
     
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
export class NapaZone implements zone.Zone {
    private _nativeZone;

    constructor(nativeZone) {
        this._nativeZone = nativeZone;
    }

    public get id(): string {
        return this._nativeZone.getId();
    }

    public broadcast(arg1: any, arg2?: any) : Promise<zone.ResponseCode> {
        let source: string;
        if (typeof arg1 === "string") {
            // broadcast with source
            source = arg1;
        } else {
            // broadcast with function
            if (!(arg1 instanceof Function)) {
                throw new TypeError("Expected a Function type argument");
            }
            
            let functionString: string = (<Function>arg1).toString();
            
            // Prepare arguments as a string
            let argumentsString: string = "";
            if (arg2 != undefined) {
                if (!(arg2 instanceof Array)) {
                    throw new TypeError("Expected an Array type argument");
                }
                argumentsString = JSON.stringify(arg2);
                argumentsString = argumentsString.substring(1, argumentsString.length - 1);
            }

            // Create a self invoking function string
            source = `(${ functionString })(${ argumentsString })`;
        }

        return new Promise<zone.ResponseCode>(resolve => {
            this._nativeZone.broadcast(source, resolve);
        });
    }

    public execute(arg1: any, arg2: any, arg3?: any, arg4?: any) : Promise<zone.ExecuteResult> {
        if (arg1 instanceof Function) {
            throw new Error("Execute with function is not implemented");
        }

        let transportContext: transport.TransportContext = transport.createTransportContext();
        let request : ExecuteRequest = {
            module: arg1,
            function: arg2,
            arguments: (<Array<any>>arg3).map(arg => { return transport.marshall(arg, transportContext); }),
            timeout: arg4,
            transportContext: transportContext
        };
        
        return new Promise<zone.ExecuteResult>((resolve, reject) => {
            this._nativeZone.execute(request, (response) => {
                if (response.code === 0) {
                    transportContext = transport.createTransportContext(response.contextHandle);
                    resolve(new NapaExecuteResult(response.returnValue, transportContext));
                } else {
                    reject(response.errorMessage);
                }
            });
        });
    }

}

