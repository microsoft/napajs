export type Handle = [number, number] | [number, number, number];

/// <summary> nullptr. </summary>
export const EMPTY_HANDLE: Handle = [0, 0];

/// <summary> Tell if pointer is nullptr. </summary>
export function isEmpty(handle: Handle): boolean {
    return handle == null ||
        (handle[0] === 0 && handle[1] === 0);
}

/// <summary> Tell if a pointer has type information. </summary>
export function getTypeHash(handle: Handle): number {
    return handle.length === 3 ? handle[2] : 0;
}
