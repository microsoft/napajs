import * as assert from 'assert';
import * as objectModel from '../lib/object-model';

describe('@napajs/vineyard/named-object', () => {
    describe('NamedObjectRegistry', () => {
        let collection = new objectModel.NamedObjectRegistry();
        let objectA: objectModel.NamedObject = {
            scope: "global",
            definition: {
                name: "objectA",
                value: 1
            },
            value: 1
        }

        it('#has', () => {
            assert(!collection.has('objectA'));
        });

        it('#insert', () => {
            collection.insert(objectA);
            assert(collection.has('objectA'));
        });

        it('#get', () => {
            let output = collection.get("objectA")
            assert.strictEqual(output, objectA);
        });

        it('#forEach', () => {
            collection.forEach((object: objectModel.NamedObject) => {
                assert.strictEqual(object, objectA);
            });
        })

        it('#fromDefinition', () => {
            let objectContext = {
                create: (input: any): any => {
                    return input;
                },
                get: (name: string): objectModel.NamedObject => {
                    return null;
                },
                forEach: (callback: (object: objectModel.NamedObject) => void) => {
                    // Do nothing.
                },
                baseDir: __dirname
            }
            collection = objectModel.NamedObjectRegistry.fromDefinition(
                "global",
                [{ name: "objectA", value: 1 }], 
                objectContext);
            assert(collection.has('objectA'));
            assert.deepEqual(collection.get('objectA'), objectA);
        })
    });
});