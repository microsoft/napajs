import * as path from 'path';

import * as utils from './utils';
import { ObjectContext } from './object-context';


//////////////////////////////////////////////////////////////////////////////////////
//  Interfaces and classes for object creation.

/// <summary> Interface for objects with '_type' property, which is used to determine object creator.
/// '_type' property is case-sensitive.
/// </summary>
export interface ObjectWithType { 
    _type: string 
};

/// <summary> Object type definition to register a type in Napa. </summary>
export interface TypeDefinition {
    typeName: string;
    description?: string;
    moduleName: string;
    functionName: string;
    override?: boolean;
    exampleObjects?: any[];
}

/// <summary> Function interface for object constructor, which takes an input object and produce an output object. </summary>
export interface ObjectConstructor { 
    (input: ObjectWithType | ObjectWithType[], context?: ObjectContext): any
}

/// <summary> Interface for object factory. </summary>
export interface ObjectFactory {

    /// <summary> Create an output JS value from input object. </summary>
    /// <param name="input"> Object with '_type' property or object array. </param>
    /// <param name="context"> Context if needed to construct sub-objects. </param>
    /// <returns> Any JS value type. </returns>
    /// <remarks>
    /// When input is array, all items in array must be the same type.
    /// On implementation, you can check whether input is array or not as Array.isArray(input).
    /// Please refer to example\example_types.ts.
    /// </remarks>
    create(input: ObjectWithType | ObjectWithType[], context?: ObjectContext): any;

    /// <summary> Check whether current object factory support given type. </summary>
    /// <param name="typeName"> value of '_type' property. </param>
    /// <returns> True if supported, else false. </returns>
    supports(typeName: string): boolean;
}

/// <summary> An implementation of ObjectFactory that allows to register type with their creator. </summary>
export class TypeRegistry implements ObjectFactory {

    /// <summary> Type name to creator map. </summary>
    private _typeToCreatorMap: Map<string, ObjectConstructor> = new Map<string, ObjectConstructor>();

    /// <summary> Create an output object from input object. Exception will be thrown if type is not found in current application. </summary>
    /// <param name="input"> Input object with a property '_type', the value of '_type' should be registered in current application. </param>
    /// <returns> Object created from input. </returns>
    /// <remarks> When input is array, all items in array must be the same type.</remarks>
    public create(input: ObjectWithType | ObjectWithType[], context?: ObjectContext): any {
        if (input == null) {
            return null;
        }
        let typeName: string;
        if (Array.isArray(input)) {
            if ((<ObjectWithType[]>input).length == 0) {
                return [];
            }
            // It assumes that all items in array are the same type.
            typeName = (<ObjectWithType[]>input)[0]._type;
            for (let elem of input) {
                if (elem._type !== typeName) {
                    throw new Error("Property '_type' must be the same for all elements in input array when calling ObjectFactory.create.");
                }
            }
        }
        else {
            typeName = (<ObjectWithType>input)._type;
        }

        if (this.supports(typeName)) {
            return this._typeToCreatorMap.get(typeName)(input, context);
        }
        throw new Error("Not supported type: '" + typeName + "'.");
    }
    
    /// <summary> Register an object creator for a type. Later call to this method on the same type will override the creator of former call.</summary>
    /// <param name="type"> Case sensitive type name.</param>
    /// <param name="creator"> Function that takes one object as input and returns an object.</param>
    public register(type: string, creator: ObjectConstructor): void {
        this._typeToCreatorMap.set(type, creator);
    }

    /// <summary> Check if current type registry contain a type.</summary>
    /// <param name="type"> Case sensitive type name. </param>
    /// <returns> True if type is registered, otherwise false. </param>
    public supports(typeName: string): boolean {
        return this._typeToCreatorMap.has(typeName);
    }

    /// <summary> Create TypeRegistry from a collection of TypeDefinitions </summary>
    /// <param name="typeDefCollection"> A collection of type definitions </param>
    /// <param name="baseDir"> Base directory name according to which module name will be resolved. </param>
    /// <returns> A TypeRegistry object. </returns>
    public static fromDefinition(typeDefCollection: TypeDefinition[], baseDir: string): TypeRegistry {
        let registry = new TypeRegistry();
        if (typeDefCollection != null) {
            typeDefCollection.forEach(def => {
                let moduleName = def.moduleName;
                if (def.moduleName.startsWith(".")) {
                    moduleName = path.resolve(baseDir, moduleName);
                }
                let creator = utils.loadFunction(moduleName, def.functionName);
                registry.register(def.typeName, creator);
            });
        }
        return registry;
    }
}
