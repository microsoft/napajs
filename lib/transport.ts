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
export function createTransportContext(handle? : Handle): TransportContext {
    return new binding.TransportContextWrap(handle);
}

export let saveFunction = functionTransporter.save;
export let loadFunction = functionTransporter.load;