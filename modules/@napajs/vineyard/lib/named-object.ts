import { ObjectContext } from './object-context';

///////////////////////////////////////////////////////////////////////////////
// Interfaces for Named Objects.
// Named Objects are application-level objects with a well-known name, so user can retrive the object via app.getObject('<name>');
//
// There are two types of named objects.
// 1) Named objects provided from JSON file per application, whose lifecycle is process- level.
// 2) Named objects provided from a single Napa request, whose lifecycle is during the request.

/// <summary> Class for named object that holds a definition and value.
/// Definition is needed to construct this named object under a different ObjectContext, e.g, from request.
/// </summary>

/// <summary> Interface for Named object definition. </summary>
export interface NamedObjectDefinition {
    name: string;
    description?: string;
    private?: boolean;
    override?: boolean;
    value: any;

    /// <summary> Dependency from current definition to object context. This is calculated automatically.</summary>
    dependencies?: ObjectContextDependency;
}

export interface NamedObject {
    /// <summary> Definition of current named object. </summary>
    definition: NamedObjectDefinition;

    /// <summary> Value of current named object </summary>
    value: any;
    
    /// <summary> Scope of where this named object is provided. </summary>
    readonly scope: string;
}

/// <summary> Interface for Named Object collection. </summary>
export interface NamedObjectCollection {
    /// <summary> Get named object by name. </summary>
    /// <param name="name"> Name. Case-sensitive. </summary>
    /// <returns> Named object if found. Otherwise undefined. </returns>
    get(name: string): NamedObject;

    /// <summary> Iterator each object in this collection. </summary>
    forEach(callback: (object: NamedObject) => void): void;
} 


/// <summary> An implementation of NamedObjectCollection based on name to object registry. </summary>
export class NamedObjectRegistry implements NamedObjectCollection {
    /// <summary> Name to object map. Case sensitive. </summary>
    private _nameToObjectMap: Map<string, NamedObject> = new Map<string, NamedObject>();

    /// <summary> Get object by name. </summary>
    /// <param name="name"> Case sensitive name. </param>
    /// <returns> undefined if not present, otherwise an instance of NamedObject. </returns>
    public get(name: string): NamedObject {
        return this._nameToObjectMap.get(name);
    }

    /// <summary> Tell if a name exists in this registry. </summary>
    /// <returns> True if exists, otherwise false. </returns>
    public has(name: string): boolean {
        return this._nameToObjectMap.has(name);
    }

    /// <summary> Iterate each object in this registry. </summary>
    public forEach(callback: (object: NamedObject) => void): void {
        this._nameToObjectMap.forEach(object => {
            callback(object);
        });
    }

    /// <summary> Insert a named object. </summary>
    /// <param name="object"> an Named object instance. </param>
    public insert(object: NamedObject): void {
        this._nameToObjectMap.set(object.definition.name, object);
    }

    /// <summary> Create NamedObjectRegistry from a collection of NamedObjectDefinition objects. </summary>
    /// <param name="scope"> Scope that current object definition apply to. Can be 'global', 'application', 'request', etc. </summary>
    /// <param name="namedObjectDefCollection"> Collection of NamedObjectDefinition objects. </param>
    /// <param name="context"> A list of ObjectContext objects. </param>
    /// <returns> NamedObjectRegistry </returns>
    public static fromDefinition(
        scope: string,
        namedObjectDefCollection: NamedObjectDefinition[],
        context: ObjectContext): NamedObjectRegistry {

        let registry = new NamedObjectRegistry();
        if (namedObjectDefCollection != null) {
            for (let def of namedObjectDefCollection) {
                let value = context.create(def.value);
                registry.insert({
                    definition: def,
                    value: value,
                    scope: scope
                });
            }
        }
        return registry;
    }
}

/// <summary> Dependency information on types, object and providers. 
/// When type, object, provider override happens at request time,
/// We use this information to determine if a named object needs to be invalidated at request time.
/// We only analyze dependency information for named objects registered at application level,
/// as request level named object anyway will be re-created.
/// </summary>
export class ObjectContextDependency {
    private _dependentTypesNames: Set<string> = new Set<string>();
    private _dependentObjectNames: Set<string> = new Set<string>(); 
    private _dependentProtocolNames: Set<string> = new Set<string>();

    /// <summary> Set a depenency on a object type </summary>
    public setTypeDependency(typeName: string) {
        this._dependentTypesNames.add(typeName);
    }

    /// <summary> Set a depenency on a URI protocol. </summary>
    public setProtocolDependency(protocolName: string) {
        this._dependentProtocolNames.add(protocolName);
    }

    /// <summary> Set a depenency on a named object. </summary>
    public setObjectDependency(objectName: string) {
        this._dependentObjectNames.add(objectName);
    }

    /// <summary> Get all dependent type names. </summary>
    public get typeDependencies(): Set<string> {
        return this._dependentTypesNames;
    }

    /// <summary> Get all dependent URI protocol names. </summary>
    public get protocolDependencies(): Set<string> {
        return this._dependentProtocolNames;
    }

    /// <summary> Get all dependent object names. </summary>
    public get objectDependencies(): Set<string> {
        return this._dependentObjectNames;
    }
};

