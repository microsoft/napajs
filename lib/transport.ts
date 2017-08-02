// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

export { 
    Transportable, 
    TransportableObject, 
    TransportContext, 
    isTransportable, 
    cid 
} from './transport/transportable';

export * from './transport/transport';

import { Handle } from './memory/handle';
import { TransportContext } from './transport/transportable';
import * as functionTransporter from './transport/function-transporter';

let binding = require('./binding');

/// <summary> Create a transport context. </summary>
export function createTransportContext(owning: boolean = true, handle : Handle = undefined): TransportContext {
    return new binding.TransportContextWrap(owning, handle);
}

export let saveFunction = functionTransporter.save;
export let loadFunction = functionTransporter.load;