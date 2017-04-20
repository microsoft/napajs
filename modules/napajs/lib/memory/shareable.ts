import { Handle} from './handle';
import * as transport from '../transport';

/// <summary> Interface of native object (wrapped shared_ptr<T>) that can be shared across isolates. </summary>
/// <remarks> see napa::module::SharedWrap. </remarks>
export interface Shareable extends transport.Transportable {
    /// <summary> Return handle of this object. </summary>
    readonly handle: Handle;

    /// <summary> Return true if handle is empty. </summary>
    isNull(): boolean;

    /// <summary> Return reference count for shared object. </summary>
    readonly refCount: number;
}

/// <summary> Tells if a JavaScript is value or not. </summary>
export function isShareable(jsValue: any): boolean {
    if (transport.isTransportable(jsValue)) {
        let object = <Object>jsValue;
        return object.hasOwnProperty('handle')
            && object.hasOwnProperty('refCount');
    }
    return false;
}
