import * as zone from "./zone";

declare var __in_napa: boolean;

/// <summary> Zone consists of Napa isolates. </summary>
export class NapaZone implements zone.Zone {
    private _nativeZone;

    constructor(nativeZone) {
        this._nativeZone = nativeZone;
    }

    public get id(): string {
        return this._nativeZone.getId();
    }

    public load(source: string) : Promise<zone.ResponseCode> {
        if (typeof __in_napa !== undefined) {
            // Napa does not support async yet, we wrap this sync call in a promise
            // to provide a uniform API.
            return Promise.resolve(this._nativeZone.loadSync(source));
        }

        return new Promise<zone.ResponseCode>(resolve => {
            this._nativeZone.load(source, resolve);
        });
    }

    public execute(module: string, func: string, args: any[], timeout?: number) : Promise<zone.ResponseValue> {
        // Convert all arguments to strings
        args = args.map(x => { return JSON.stringify(x); });

        if (typeof __in_napa !== undefined) {
            // Napa does not support async yet, we wrap this sync call in a promise
            // to provide a uniform API.
            return Promise.resolve(this._nativeZone.executeSync(module, func, args, timeout));
        }

        return new Promise<zone.ResponseValue>((resolve, reject) => {
            this._nativeZone.execute(module, func, args, (response) => {
                if (response.code === 0) {
                    resolve(response.returnValue);
                } else {
                    reject(response.errorMessage);
                }
            }, timeout);
        });
    }

    public executeAll(module: string, func: string, args: any[]) : Promise<zone.ResponseCode> {
        // Convert all arguments to strings
        args = args.map(x => { return JSON.stringify(x); });
        
        if (typeof __in_napa !== undefined) {
            // Napa does not support async yet, we wrap this sync call in a promise
            // to provide a uniform API.
            return Promise.resolve(this._nativeZone.executeAllSync(module, func, args));
        }

        return new Promise<zone.ResponseCode>(resolve => {
            this._nativeZone.executeAll(module, func, args, resolve);
        });
    }

    public destory(): void {
        this._nativeZone.destroy();
    }
}

