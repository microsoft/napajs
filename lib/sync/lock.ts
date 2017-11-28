// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

let binding = require('../binding');
import { cid, TransportContext, TransportableObject } from '../transport';

import * as v8 from '../v8';

/// <summary> Class Lock offers ability for exclusive execution for sync and async functions cross multiple threads. </summary>
@cid()
export class Lock extends TransportableObject {

    /// <summary> Defualt constructor </summary>
    constructor() {
        super();
        this._lock = binding.createLock();
    }

    save(payload: object, context: TransportContext): void {
        (<any>payload)['_lock'] = this._lock.marshall(context);
    }
    load(payload: object, context: TransportContext): void {
    }

    /// <summary>
    /// Obtain the lock and input function synchronously.
    /// or throws error if input function throws.
    /// Lock will be released once execution finishes or an exception is thrown.
    /// </summary>
    /// <param name="func"> The input function to run. </summary>
    /// <returns> The value that the input function returns. </returns>
    /// <remarks> This function will obtain the lock before running the input function. It will wait until the
    /// lock is available. If the input function throws exception, the exception will be thrown out. </remarks>
    guardSync(func: () => any): any {
        this._lock.enter();

        try {
            let result: any = func();
            this._lock.exit();
            return result;
        }
        catch (error) {
            this._lock.exit();
            throw error;
        }
    }

    private _lock: any;
}