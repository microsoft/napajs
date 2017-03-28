import * as assert from 'assert';
import * as objectModel from '../lib/object-model';

describe('@napajs/vineyard/object-context', () => {
    let perAppDef: objectModel.ScopedObjectContextDefinition = null;
    let perRequestDef: objectModel.ScopedObjectContextDefinition = null;

    // Test suite for ScopedObjectContextDefinition.
    describe('ScopedObjectContextDefinition', () => {
        // Per app definitions.
        let perAppTypeDefs: objectModel.TypeDefinition[] = [
            {
                typeName: "TypeA",
                moduleName: "./object-context-test",
                functionName: "types.createTypeA"
            },
            {
                typeName: "TypeB",
                moduleName: "./object-context-test",
                functionName: "types.createTypeB"
            }
        ];

        let perAppProviderDefs: objectModel.ProviderDefinition[] = [
            {
                protocol: "ProtocolA",
                moduleName: "./object-context-test",
                functionName: "provideProtocolA"
            }
        ];

        let perAppObjectDefs: objectModel.NamedObjectDefinition[] = [
            {
                name: "objectA",
                value: {
                    _type: "TypeA",
                    value: 1
                }
            },
            {
                name: "objectB",
                value: {
                    _type: "TypeB",
                    value: {
                        _type: "TypeA",
                        value: 1
                    }
                }
            },
            {
                name: "objectC",
                value: "ProtocolA://abc"
            },
        ];

        // Per request definitions.
        let perRequestTypeDefs: objectModel.TypeDefinition[] = [
            {
                typeName: "TypeA",
                moduleName: "./object-context-test",
                functionName: "types.createTypeAPrime"
            }
        ];

        let perRequestProviderDefs: objectModel.ProviderDefinition[] = [
            {
                protocol: "ProtocolA",
                moduleName: "./object-context-test",
                functionName: "provideProtocolAPrime"
            }
        ];

        let perRequestObjectDefs: objectModel.NamedObjectDefinition[] = [
            {
                name: "objectA",
                value: {
                    _type: "TypeA",
                    value: 2
                }
            },
            {
                name: "objectC",
                value: "ProtocolA://cde"
            },
            {
                name: "objectD",
                value: "ProtocolA://def"
            }
        ];
        
        it("#ctor", () => {
            assert.doesNotThrow(() => {
                perAppDef = new objectModel.ScopedObjectContextDefinition(null, perAppTypeDefs, perAppProviderDefs, perAppObjectDefs, true);
                perRequestDef = new objectModel.ScopedObjectContextDefinition(perAppDef, perRequestTypeDefs, perRequestProviderDefs, perRequestObjectDefs, false);
            });
        });

        it('#getters', () => {
            assert.strictEqual(perAppDef.parent, null);
            assert.strictEqual(perAppDef.types, perAppTypeDefs);
            assert.strictEqual(perAppDef.providers, perAppProviderDefs);
            assert.strictEqual(perAppDef.namedObjects, perAppObjectDefs);
            
            assert.strictEqual(perAppDef.getType('TypeA'), perAppTypeDefs[0]);
            assert.strictEqual(perAppDef.getType('TypeB'), perAppTypeDefs[1]);
            assert.strictEqual(perAppDef.getProvider('ProtocolA'), perAppProviderDefs[0]);

            assert.strictEqual(perAppDef.getNamedObject('objectA'), perAppObjectDefs[0]);
            assert.strictEqual(perAppDef.getNamedObject('objectB'), perAppObjectDefs[1]);
            assert.strictEqual(perAppDef.getNamedObject('objectC'), perAppObjectDefs[2]);
        });

        it('#analyzeDependency', () => {
            // objectA
            let dep1 = perAppObjectDefs[0].dependencies;
            assert(dep1.objectDependencies.size == 0);
            assert(dep1.protocolDependencies.size == 0);
            assert(dep1.typeDependencies.size == 1 && dep1.typeDependencies.has('TypeA'));

            // objectB
            let dep2 = perAppObjectDefs[1].dependencies;
            assert(dep2.objectDependencies.size == 0);
            assert(dep2.protocolDependencies.size == 0);
            assert(dep2.typeDependencies.size == 2 
                && dep2.typeDependencies.has('TypeA')
                && dep2.typeDependencies.has('TypeB'));

            // objectC
            let dep3 = perAppObjectDefs[2].dependencies;
            assert(dep3.objectDependencies.size == 0);
            assert(dep3.protocolDependencies.size == 1 && dep3.protocolDependencies.has('ProtocolA'));
            assert(dep3.typeDependencies.size == 0);
        });
    });

    // Test suite for ScopedObjectContext
    describe('ScopedObjectContext', () => {
        let perAppContext: objectModel.ScopedObjectContext = null;
        let perRequestContext: objectModel.ScopedObjectContext = null;
        
        it('#ctor', () => {
            perAppContext = new objectModel.ScopedObjectContext("application", __dirname, null, perAppDef);
            perRequestContext = new objectModel.ScopedObjectContext("request", __dirname, perAppContext, perRequestDef);
        });

        it('#getters', () => {
            assert.strictEqual(perAppContext.scope, "application");
            assert.strictEqual(perAppContext.baseDir, __dirname);
            assert.strictEqual(perAppContext.definition, perAppDef);
            assert.strictEqual(perAppContext.parent, null);
            assert.strictEqual(perRequestContext.parent, perAppContext);
        });

        it('#create: overridden TypeA', () => {
            let inputA = { _type: "TypeA", value: 1};
            assert.strictEqual(perAppContext.create(inputA), 1);
            assert.strictEqual(perRequestContext.create(inputA), 2);
        });

        it('#create: not overridden TypeB', () => {
            let inputB = { _type: "TypeB", value: { _type: "TypeA", value: 1}};
            assert.strictEqual(perAppContext.create(inputB), 1);
            // B returns A's value, which is different from per-app and per-request.
            assert.strictEqual(perRequestContext.create(inputB), 2);
        });

        it('#create: overridden ProtocolA', () => {
            let uri = "ProtocolA://abc";
            assert.strictEqual(perAppContext.create(uri), "/abc");
            assert.strictEqual(perRequestContext.create(uri), "/abc*");
        });

        it('#get: overriden objectA', () => {
            let objectA = perAppContext.get('objectA');
            assert.strictEqual(objectA.scope, 'application');
            assert.strictEqual(objectA.value, 1);

            objectA = perRequestContext.get('objectA');
            assert.strictEqual(objectA.scope, 'request');
            assert.strictEqual(objectA.value, 3);
        });

        it('#get: not overridden objectB', () => {
            let objectB = perAppContext.get('objectB');
            assert.strictEqual(objectB.scope, 'application');
            assert.strictEqual(objectB.value, 1);

            objectB = perRequestContext.get('objectB');
            assert.strictEqual(objectB.scope, 'application');
            assert.strictEqual(objectB.value, 1);
        });

        it('#get: overriden objectC with new providerA', () => {
            let objectC = perAppContext.get('objectC');
            assert.strictEqual(objectC.scope, 'application');
            assert.strictEqual(objectC.value, '/abc');

            objectC = perRequestContext.get('objectC');
            assert.strictEqual(objectC.scope, 'request');
            assert.strictEqual(objectC.value, '/cde*');
        });

        it('#get: new objectD with new providerA', () => {
            let objectD = perAppContext.get('objectD');
            assert(objectD == null);

            objectD = perRequestContext.get('objectD');
            assert.strictEqual(objectD.scope, 'request');
            assert.strictEqual(objectD.value, '/def*');
        });

        it('#forEach: without parent scope', () => {
            let objectNames: string[] = []
            perAppContext.forEach(object => {
                objectNames.push(object.definition.name);
            });
            assert.strictEqual(objectNames.length, 3);
            assert(objectNames.indexOf('objectA') >= 0);
            assert(objectNames.indexOf('objectB') >= 0);
            assert(objectNames.indexOf('objectC') >= 0);
        });

        it('#forEach: with parent scope', () => {
            let objectCount = 0;
            let objectByName = new Map<string, objectModel.NamedObject>();
            perRequestContext.forEach(object => {
                ++objectCount;
                objectByName.set(object.definition.name, object);
                if (object.definition.name !== 'objectB') {
                    assert(object.scope === 'request');
                }
            });
            assert.strictEqual(objectCount, 4);
            assert.strictEqual(objectByName.size, objectCount);
            assert(objectByName.has('objectA') && objectByName.get('objectA').scope === 'request');
            assert(objectByName.has('objectB') && objectByName.get('objectB').scope === 'application');
            assert(objectByName.has('objectC') && objectByName.get('objectC').scope === 'request');
            assert(objectByName.has('objectD') && objectByName.get('objectD').scope === 'request');
        });

        // TODO: Add test for needsUpdate.
        it('#needsUpdate');
    });
});

export type TypeAInput = { _type: "TypeA", value: number};

// Test calling function with 
export namespace types {
    export function createTypeA(input: TypeAInput | TypeAInput[]) {
        if (Array.isArray(input)) {
            return input.map(elem => elem.value);
        }
        return input.value;
    }

   export function createTypeAPrime(input: TypeAInput | TypeAInput[]) {
        if (Array.isArray(input)) {
            return input.map(elem => elem.value + 1);
        }
        return input.value + 1;
    }

    export function createTypeB(input: TypeBInput, context: objectModel.ObjectContext) {
        return context.create(input.value);
    }
}

export type TypeBInput = { _type: "TypeB", value: TypeAInput};

export function provideProtocolA(uri: objectModel.Uri): any {
    return uri.path;
}

export function provideProtocolAPrime(uri: objectModel.Uri): any {
    return uri.path + '*';
}