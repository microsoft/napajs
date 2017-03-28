import * as assert from 'assert';
import * as path from 'path';

import * as objectModel from '../lib/object-model';

describe('@napajs/vineyard/object-provider', () => {
    describe('Uri', () => {
        it('#parse: absolute path with no parameters.', () => {
            let uri = objectModel.Uri.parse("doc://a/d/e/f");
            assert.equal(uri.path, "/a/d/e/f");
            assert.equal(uri.protocol, "doc");
        });

        it('#parse: relative path with no parameters.', () => {
            let uri = objectModel.Uri.parse("doc:/a/d/e/f");
            assert.equal(uri.path, "a/d/e/f");
            assert.equal(uri.protocol, "doc");
        });

        it('#parse: absolute path with parameters.', () => {
            let uri = objectModel.Uri.parse("doc://a/d/e/f?a=1&b=2");
            assert.equal(uri.path, "/a/d/e/f");
            assert.equal(uri.protocol, "doc");
            assert.strictEqual(uri.getParameter("a"), "1");
            assert.strictEqual(uri.getParameter("b"), "2");
        });
        
        it('#parse: bad format.', () => {
            assert.throws(() => {
                objectModel.Uri.parse("doc//a/d/e/f?a=1&b=2");
            });
        });
    });

    describe('ProviderRegistry', () => {
        let provider = new objectModel.ProviderRegistry();
        
        // ProtocolA support both a single element and an array as input.
        it('#register', () => {
            provider.register('protocolA', 
                (uri: objectModel.Uri | objectModel.Uri[]): string | string[] => {
                    if (Array.isArray(uri)) {
                        return uri.map(value => { return value.path; });
                    }
                    return uri.path;
                });

            // ProtocolB needs an ObjectContext to create inner object.
            provider.register('protocolB',
                    (input: objectModel.Uri, context: objectModel.ObjectContext): any => {
                        return path.resolve(context.baseDir, input.path);
                    });
        });

        it('#supports', () => {
            // Case insensitive.
            assert(provider.supports('protocolA'));
            assert(provider.supports('ProtocolA'));
            assert(provider.supports('protocola'));

            assert(provider.supports('protocolB'));
            assert(!provider.supports('protocolC'));
        });
        
        it('#provide: unsupported protocol', () => {
            // Create object of unsupported type.
            assert.throws(() => { 
                    provider.provide(objectModel.Uri.parse("protocolC://abc"));
                }, 
                Error);
        });

        let uriA1 = objectModel.Uri.parse("protocolA://abc");
        let expectedA1 = "/abc";
        it('#provide: input with single uri', () => {
            // Create object with a single uri. 
            let a1 = provider.provide(uriA1);
            assert.strictEqual(a1, expectedA1);
        });

        it('#provide: case insensitive protocol', () => {
            // Create object with a single uri. 
            let a1 = provider.provide(objectModel.Uri.parse("PrOtOcOlA://abc"));
            assert.strictEqual(a1, expectedA1);
        });
        
        it('#provide: input with array of uri.', () => {
            // Create an array of objects with an array of uris.
            let uriA2 = objectModel.Uri.parse("protocolA://cde");
            let arrayA = provider.provide([uriA1, uriA2]);
            assert.deepEqual(arrayA, ["/abc", "/cde"]);
        });

        // Create an object that needs ObjectContext.
        // Create a simple context.
        var context: objectModel.ObjectContext = {
            create: (input: any): any => {
                return null;
            },
            get: (name: string): objectModel.NamedObject => {
                return null;
            },
            forEach: (callback: (object: objectModel.NamedObject) => void) => {
                // Do nothing.
            },
            baseDir: __dirname
        }

        let uriB1 = objectModel.Uri.parse("protocolB:/file1.txt");
        it('#provide: protocol needs object context', () => {
            assert.equal(provider.provide(uriB1, context), path.resolve(__dirname, "file1.txt"));
        });

        it('#provide: mixed protocol in a Uri array', () => {
            // Create an array of objects of different protocol. 
            assert.throws(() => {
                    provider.provide([uriA1, uriB1], context);
                }, Error);
        });

        it('ProviderRegistry#fromDefinition', () => {
            let defs: objectModel.ProviderDefinition[] = [{
                protocol: "protocolA",
                moduleName: "./object-provider-test",
                functionName: "loadA"
            }];
            let provider = objectModel.ProviderRegistry.fromDefinition(defs, __dirname);
            assert(provider.supports('protocolA'));
            
            let uriA1 = objectModel.Uri.parse("protocolA://abc")
            assert.equal(provider.provide(uriA1), "/abc");
        });
    });
});

export function loadA(uri: objectModel.Uri): string {
    return uri.path;
}