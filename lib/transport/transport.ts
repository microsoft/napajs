// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

import * as transportable from './transportable';
import * as functionTransporter from './function-transporter';
import * as builtinObjectTransporter from './builtin-object-transporter';
import * as path from 'path';

/// <summary> Per-isolate cid => constructor registry. </summary>
let _registry: Map<string, new(...args: any[]) => transportable.Transportable> 
    = new Map<string, new(...args: any[]) => transportable.Transportable>();

let _builtInTypeWhitelist = new Set();
[
    'ArrayBuffer',
    'Float32Array',
    'Float64Array',
    'Int16Array',
    'Int32Array',
    'Int8Array',
    'SharedArrayBuffer',
    'Uint16Array',
    'Uint32Array',
    'Uint8Array'
].forEach((type) => { _builtInTypeWhitelist.add(type); });

/// <summary> Register a TransportableObject sub-class with a Constructor ID (cid). </summary>
export function register(subClass: new(...args: any[]) => any) {
    // Check cid from constructor first, which is for TransportableObject. 
    // Thus we don't need to construct the object to get cid according to Transportable interface. 
    let cid: string = (<any>subClass)['_cid'];
    if (cid == null) {
        cid = new subClass().cid();
    }
    if (cid == null) {
        throw new Error(`Class "${subClass.name}" doesn't implement cid(), did you forget put @cid decorator before class declaration?`);
    }
    if (_registry.has(cid)) {
        throw new Error(`Constructor ID (cid) "${cid}" is already registered.`);
    }
    _registry.set(cid, subClass);
}

/// <summary> Marshall transform a JS value to a plain JS value that will be stringified. </summary> 
export function marshallTransform(jsValue: any, context: transportable.TransportContext): any {
     if (jsValue != null && typeof jsValue === 'object' && !Array.isArray(jsValue)) {
        let constructorName = Object.getPrototypeOf(jsValue).constructor.name;
        if (constructorName !== 'Object') {
            if (typeof jsValue['cid'] === 'function') {
                return <transportable.Transportable>(jsValue).marshall(context);
            } else if (_builtInTypeWhitelist.has(constructorName)) {
                let serializedData = builtinObjectTransporter.serializeValue(jsValue);
                if (serializedData) {
                    return { _serialized : serializedData };
                } else {
                    throw new Error(`Failed to serialize object with type of \"${constructorName}\".`);
                }
            } else {
                throw new Error(`Object type \"${constructorName}\" is not transportable.`);
            }
        }
    }
    return jsValue;
}

/// <summary> Unmarshall transform a plain JS value to a transportable class instance. </summary>
/// <param name="payload"> Plain Javascript value. </param> 
/// <param name="context"> Transport context. </param>
/// <returns> Transported value. </returns>
function unmarshallTransform(payload: any, context: transportable.TransportContext): any {
    if (payload == null) return payload;
    if (payload._cid !== undefined) {
        let cid = payload._cid;
        if (cid === 'function') {
            return functionTransporter.load(payload.hash);
        }
        let subClass = _registry.get(cid);
        if (subClass == null) {
            throw new Error(`Unrecognized Constructor ID (cid) "${cid}". Please ensure @cid is applied on the class or transport.register is called on the class.`);
        }
        let object = new subClass();
        object.unmarshall(payload, context);
        return object;
    } else if (payload.hasOwnProperty('_serialized')) {
        return builtinObjectTransporter.deserializeValue(payload['_serialized']);
    }
    return payload;
}

/// <summary> Unmarshall from JSON string to a JavaScript value, which could contain complex/native objects. </summary>
/// <param name="json"> JSON string. </summary>
/// <param name="context"> Transport context to save shared pointers. </param>
/// <returns> Parsed JavaScript value, which could be built-in JavaScript types or deserialized Transportable objects. </returns>
export function unmarshall(
    json: string, 
    context: transportable.TransportContext): any {
    
    if (json === "undefined") {
        return undefined;
    }
    return JSON.parse(json, 
        (key: any, value: any): any => {
            return unmarshallTransform(value, context);
        });
}

/// <summary> Marshall a JavaScript value to JSON. </summary>
/// <param name="jsValue"> JavaScript value to stringify, which maybe built-in JavaScript types or transportable objects. </param>
/// <param name="context"> Transport context to save shared pointers. </param>
/// <returns> JSON string. </returns>
export function marshall(
    jsValue: any, 
    context: transportable.TransportContext): string {

    // Function is transportable only as root object. 
    // This is to avoid unexpected marshalling on member functions.
    if (typeof jsValue === 'function') {
        return `{"_cid": "function", "hash": "${functionTransporter.save(jsValue)}"}`;
    }
    return JSON.stringify(jsValue,
        (key: string, value: any) => {
            return marshallTransform(value, context);
        });
}