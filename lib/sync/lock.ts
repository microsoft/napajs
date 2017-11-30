// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

let binding = require('../binding');
import { cid, TransportContext, TransportableObject } from '../transport';

export interface Lock {
    /// <summary>
    /// Obtain the lock and input function synchronously.
    /// or throws error if input function throws.
    /// Lock will be released once execution finishes or an exception is thrown.
    /// </summary>
    /// <param name="func"> The input function to run. </summary>
    /// <returns> The value that the input function returns. </returns>
    /// <remarks> This function will obtain the lock before running the input function. It will wait until the
    /// lock is available. If the input function throws exception, the exception will be thrown out. </remarks>
    guardSync(func: () => any): any;
}

export function createLock(): Lock {
    return binding.createLock();
}