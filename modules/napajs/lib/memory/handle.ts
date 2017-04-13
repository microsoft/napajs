import * as transportable from '../transport/transportable'

/// <summary> Handle value, which is transport format of Handle.
/// The first two uint32 integer are the high-32 bits and low-32 bits of a 64-bit pointer.
/// The optional 3rd integer is type identifier, which is computed via hash of std::type_index % UINT32_MAX.
/// </summary>
export type HandleValue = [number, number] | [number, number, number];

/// <summary> nullptr. </summary>
export const EMPTY_HANDLE_VALUE: HandleValue = [0, 0];

/// <summary> Tell if pointer is nullptr. </summary>
export function isEmpty(handleValue: HandleValue): boolean {
    return handleValue == null ||
        (handleValue[0] === 0 && handleValue[1] === 0);
}

/// <summary> Tell if a pointer has type information. </summary>
export function getTypeHash(pointer: HandleValue): number {
    return pointer.length === 3 ? pointer[2] : 0;
}

/// <summary> Handle is a Napa.JS concept to share C++ objects across V8 isolates.
/// Interface Handle describes the common properties and methods we exposed addon 'HandleWrap' and its descedants.
/// A handle instance can be casted into napa::memory::Handle<T> in C++ language. </summary>
export interface Handle extends transportable.Transportable {
    /// <summary> cid used to transport Handle, always return to '<Handle>' </summary>
    readonly cid: string;

    /// <summary> Return value of this handle. </summary>
    readonly value: HandleValue;

    /// <summary> Return true if this handle is empty. </summary>
    isEmpty(): boolean;

    /// <summary> Return if this handle is a void pointer. </summary>
    isVoid(): boolean;

    /// <summary> Marshall handle to object. </summary>
    marshall(context: transportable.TransportContext): object;

    /// <summary> Unmarshall handle from payload that carries with 'value' property. . </summary>
    unmarshall(payload: object, context: transportable.TransportContext): void;
}

/// <summary> Tells if a JavaScript is value or not. </summary>
export function isHandle(jsValue: any): boolean {
    // TODO:
    return false;
}
