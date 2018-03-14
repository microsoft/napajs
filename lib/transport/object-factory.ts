

/// <summary> Per-isolate cid => constructor registry. </summary>
let _registry: Map<string, new(...args: any[]) => any> 
    = new Map<string, new(...args: any[]) => any>();

/// <summary> Register a TransportableObject sub-class with a Constructor ID (cid). </summary>
export function registerConstructor(cid: string, objectClass: new(...args: any[]) => any) {
    if (_registry.has(cid)) {
        throw new Error(`Constructor ID (cid) "${cid}" is already registered.`);
    }
    _registry.set(cid, objectClass);
}


/// <summary> Register a TransportableObject sub-class with a Constructor ID (cid). </summary>
export function getConstructor(cid: string) {
    let objectClass = _registry.get(cid);
    if (objectClass == null) {
        throw new Error(`Unrecognized Constructor ID (cid) "${cid}". Please ensure @cid is applied on the class or transport.register is called on the class.`);
    }
    return objectClass;
}

/// <summary> Register a TransportableObject sub-class with a Constructor ID (cid). </summary>
export function newInstance(cid: string) {
    let objectClass = _registry.get(cid);
    if (objectClass == null) {
        throw new Error(`Unrecognized Constructor ID (cid) "${cid}". Please ensure @cid is applied on the class or transport.register is called on the class.`);
    }
    let object = new objectClass();
    return object;
}