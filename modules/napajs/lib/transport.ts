export * from './transport/transportable';
export * from './transport/non-transportable';
export * from './transport/transport';

import { Handle } from './memory/handle';
import { TransportContext } from './transport/transportable';

var addon = require('../bin/addon');

/// <summary> Create a transport context. </summary>
export function createTransportContext(handle? : Handle): TransportContext {
    return new addon.TransportContextWrap(handle);
}