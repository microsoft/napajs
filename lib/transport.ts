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

let binding = require('./binding');

/// <summary> Create a transport context. </summary>
export function createTransportContext(handle? : Handle): TransportContext {
    return new binding.TransportContextWrap(handle);
}