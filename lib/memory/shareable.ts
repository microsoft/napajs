// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

import { Handle} from './handle';
import { Transportable, isTransportable } from '../transport/transportable';

/// <summary> Interface of native object (wrapped shared_ptr<T>) that can be shared across isolates. </summary>
/// <remarks> The counterpart of Shareable is napa::module::ShareableWrap, always extend napa::module::ShareableWrap for sharable native object. </remarks>
export interface Shareable extends Transportable {
    /// <summary> Return handle of this object. </summary>
    readonly handle: Handle;

    /// <summary> Return true if handle is empty. </summary>
    isNull(): boolean;

    /// <summary> Return reference count for shared object. </summary>
    readonly refCount: number;
}

/// <summary> Tells if a JavaScript is value or not. </summary>
export function isShareable(jsValue: any): boolean {
    if (isTransportable(jsValue)) {
        let object = <Object>jsValue;
        return object.hasOwnProperty('handle')
            && object.hasOwnProperty('refCount');
    }
    return false;
}
