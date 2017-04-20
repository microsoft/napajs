import { Handle } from './handle';
import { Shareable } from './shareable';
import { TransportContext } from '../transport';

/// <summary> SharedWrap is a C++ module that wraps shared_ptr<T>. </summary>
declare class SharedWrap implements Shareable {

    /// <summary> Get Constructor ID (cid) for transportable object. </summary>
    cid(): string;

    /// <summary> Marshall object into plain JavaScript object. </summary>
    /// <param name='context'> Transport context for saving shared pointers, only usable for C++ addons implemented SharedWrap. </param>
    /// <returns> Plain JavaScript value. </returns>
    marshall(context: TransportContext): object;

    /// <summary> Unmarshall object from payload, which is a plain JavaScript value, and a transport context. </summary>
    /// <param name='payload'> Payload to read from, which already have inner objects transported. </param>
    /// <param name='context'> Transport context for loading shared pointers, only needed for C++ addons implemented SharedWrap </param>
    unmarshall(payload: object, context: TransportContext): void
    
    /// <summary> Return handle of this object. </summary>
    readonly handle: Handle;

    /// <summary> Return true if handle is empty. </summary>
    isNull(): boolean;

    /// <summary> Return reference count for shared object. </summary>
    readonly refCount: number;
}