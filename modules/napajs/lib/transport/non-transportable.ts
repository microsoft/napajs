export const CID = '<non-transportable>';

/// <summary> Class decorator 'nontransportable' that indicates a class cannot be transported between isolates. </summary>
export function nontransportable<T extends { new(...args: any[]): {}}>(
    constructor: T) {
    return class extends constructor {
        static _cid: string = CID;
    };
}

/// <summary> Tell if a jsValue is decorated by @nontransportable. </summary>
export function isNonTransportable(jsValue: any): boolean {
    return jsValue != null 
        && jsValue instanceof Object
        && Object.getPrototypeOf(jsValue).constructor._cid === CID;
}
