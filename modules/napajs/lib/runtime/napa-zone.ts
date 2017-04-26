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

/// <summary> Zone consists of Napa isolates. </summary>
export class NapaZone implements zone.Zone {
    private _nativeZone;

    constructor(nativeZone) {
        this._nativeZone = nativeZone;
    }

    public get id(): string {
        return this._nativeZone.getId();
    }

    public broadcast(source: string) : Promise<zone.ResponseCode> {
        if (typeof __in_napa !== undefined) {
            // Napa does not support async yet, we wrap this sync call in a promise
            // to provide a uniform API.
            return Promise.resolve(this._nativeZone.broadcastSync(source));
        }

        return new Promise<zone.ResponseCode>(resolve => {
            this._nativeZone.load(source, resolve);
        });
    }

    public execute(module: string, func: string, args: any[], timeout?: number) : Promise<zone.ResponseValue> {        
        let transportContext: transport.TransportContext = transport.createTransportContext();
        let request : ExecuteRequest = {
            module: module,
            function: func,
            arguments: args.map(arg => { return transport.marshall(arg, transportContext); }),
            timeout: timeout,
            transportContext: transportContext
        };

        if (typeof __in_napa !== undefined) {
            // Napa does not support async yet, we wrap this sync call in a promise
            // to provide a uniform API.
            let response = this._nativeZone.executeSync(request);
            if (response.code === 0) {
                transportContext = transport.createTransportContext(response.contextHandle);
                return Promise.resolve(transport.unmarshall(response.returnValue, transportContext));
            } else {
                return Promise.reject(response.errorMessage);
            }
        }

        return new Promise<zone.ResponseValue>((resolve, reject) => {
            this._nativeZone.execute(module, func, args, (response) => {
                if (response.code === 0) {
                    transportContext = transport.createTransportContext(response.contextHandle);
                    resolve(transport.unmarshall(response.returnValue, transportContext));
                } else {
                    reject(response.errorMessage);
                }
            }, timeout);
        });
    }

}

