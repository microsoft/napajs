/// <summary> In Napa.JS, transporting objects across isolates is required for multi-thread collaborations.
/// 
/// A JavaScript value is transportable, if
/// 1) it is a built-in JavaScript types, which includes primitive types, plain object and arrays.
/// 2) it extends TransportableObject.
/// 3) it is a container (array of object) that only contain built-in JavaScript types or objects that implemented Transportable interface.
/// 
/// </summary>

import * as nontransportable from './non-transportable';
import * as transport from './transport'

import { Shareable } from '../memory/shareable';
import { Handle } from '../memory/handle';

import * as path from 'path';

/// <summary> Transport context carries additional information needed to unmarshall 
/// objects, besides the payload itself. Currently, only std::shared_ptr<T> is transported via TransportContext,
/// since their lifecycle are beyond an V8 isolate thus cannot be managed only via payload.
/// TransportContext is a C++ add-on and only accessed from C++ thus no method is exposed to JavaScript world.
/// </summary>
/// <remarks> Reference: napa::binding::TransportContextWrapImpl. </remarks>
export interface TransportContext {
    /// <summary> Save a shared_ptr for later load in another isolate. </summary>
    saveShared(object: Shareable): void;

    /// <summary> Load a shared_ptr from previous save in another isolate. </summary>
    loadShared(handle: Handle): Shareable;

    /// <summary> Number of shared object saved in this TransportContext. </summary> 
    readonly sharedCount: number;
}

/// <summary> Interface for transportable non-built-in JavaScript types. </summary> 
export interface Transportable {
    /// <summary> Get Constructor ID (cid) for transportable object. </summary>
    cid(): string;

    /// <summary> Marshall object into plain JavaScript object. </summary>
    /// <param name='context'> Transport context for saving shared pointers, only usable for C++ addons that extends napa::module::SharedWrap. </param>
    /// <returns> Plain JavaScript value. </returns>
    marshall(context: TransportContext): object;

    /// <summary> Unmarshall object from payload, which is a plain JavaScript value, and a transport context. </summary>
    /// <param name='payload'> Payload to read from, which already have inner objects transported. </param>
    /// <param name='context'> Transport context for loading shared pointers, only needed for C++ addons that extends napa::module::SharedWrap </param>
    unmarshall(payload: object, context: TransportContext): void
}

/// <summary> Abstract class for transportable objects. 
/// Subclass' obligations:
/// 1) Constructor should accept zero parameters .
/// 2) Implement save()/load() to marshall/unmarshall internal state.
/// 3) Register with transport with a Constructor ID (cid) via one of following methods:
///    - declare class decorator: @cid(module.id) for exported classes.
///    - declare class decorator: @cid(module.id, '<class-name>') for non-exported classes.
///    - explicitly call transport.register(cid, <class-name>).
/// </summary>
export abstract class TransportableObject implements Transportable{
    /// <summary> Get Constructor ID (cid) for this object. </summary>
    cid(): string {
        return Object.getPrototypeOf(this)._cid;
    }

    /// <summary> Subclass to save state to payload. </summary>
    /// <param name='payload'> Payload to write to. 
    /// The sub-class should always serialize states into the payload 
    /// if they are required to load the sub-class instance. </param>
    /// <param name='context'> Transport context for saving shared pointers, only usable for C++ addons that extends napa::module::SharedWrap. </param>
    abstract save(payload: object, context: TransportContext): void;

    /// <summary> Subclass to load state from payload. </summary>
    /// <param name='payload'> Payload to read from, which already have inner objects transported. </param>
    /// <param name='context'> Transport context for loading shared pointers, only usable for C++ addons that extends napa::module::SharedWrap. </param>
    abstract load(payload: object, context: TransportContext): void;

    /// <summary> Marshall object into plain JavaScript object. </summary>
    /// <returns> Plain JavaScript value. </returns>
    marshall(context: TransportContext): object {
        let payload = {
            _cid: this.cid
        };
        this.save(payload, context);
        return payload;
    } 

    /// <summary> Unmarshall object from payload, which is a plain JavaScript value, and a transport context. </summary>
    /// <param name='payload'> Payload to read from, which already have inner objects transported. </param>
    /// <param name='context'> Transport context for looking up shared pointers. </param>
    unmarshall(payload: object, context: TransportContext): void {
        this.load(payload, context);
    }
}

/// <summary> Tell if a jsValue is a TransportableObject. </summary>
export function isTransportable(jsValue: any): boolean {
    return jsValue != null 
        && jsValue instanceof Object
        && typeof jsValue['cid'] === 'function'
        && jsValue.cid() !== nontransportable.CID;
}

/// <summary> Decorator 'cid' to register a transportable class with a 'cid'. </summary>
/// <param name="moduleId"> Return value of 'module.id' within sub-class definition file. </param>
/// <param name="className"> Optional for exported class, which will use class name by default. 
/// For non-exported class, it is required, otherwise 'Object' will be used as className. </param>
export function cid<T extends TransportableObject>(moduleId: string, className?: string) {
    let cid = className;
    if (moduleId != null && moduleId.length !== 0) {
        let moduleName = extractModuleName(moduleId);
        if (className == null) {
            className = Object.getPrototypeOf(this).constructor.name; 
        }
        cid = `${moduleName}.${className}`; 
    }

    return (constructor: new(...args: any[]) => any ) => {
        (<any>constructor)['_cid'] = cid;
        transport.register(constructor);
    }
}

const NODE_MODULE_PREFIX: string = 'node_modules/';

/// <summary> Extract module name from module.id.</summary>
function extractModuleName(moduleId: string): string {
    moduleId = moduleId.replace('\\\\', '/');
    if (moduleId.endsWith('.js')) {
        moduleId = moduleId.substr(0, moduleId.length - 3);
    }

    let moduleRootStart = moduleId.lastIndexOf(NODE_MODULE_PREFIX);
    if (moduleRootStart >= 0) {
        // module is under node_modules.
        return moduleId.substr(moduleRootStart + NODE_MODULE_PREFIX.length);
    }
    else {
        // module is located using absoluate or relative path.
        return path.relative(process.cwd(), moduleId).replace('\\\\', '/');
    }
}