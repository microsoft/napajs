// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

import * as assert from "assert";
import * as path from "path";
import * as napa from "../lib/index";

type Zone = napa.zone.Zone;

function shouldFail<T>(func: () => Promise<T>) {
    return func().then(
        (value: T) => {
            assert(false, "Failure was expected.");
        }, 
        (reason: any) => { 
            // Swallow the rejection since we expect failure
        });
}

describe('napajs/zone', function () {
    let napaZone1: Zone = napa.zone.create('napa-zone1');
    let napaZone2: Zone = napa.zone.create('napa-zone2');
    let napaLibPath: string = path.resolve(__dirname, '../lib');
    
    describe('create', () => {
        it('@node: default settings', () => {
            assert(napaZone1 != null);
            assert.strictEqual(napaZone1.id, 'napa-zone1');
        });

        // This case may be slow as the first hit of napa zone execute API, so we clear timeout.
        it('@napa: default settings', async () => {
            // Zone should be destroyed when going out of scope.
            let result = await napaZone1.execute(`${napaLibPath}/zone`, 'create', ['new-zone']);
            assert.equal(result.value.id, "new-zone");
        }).timeout(0);

        it('@node: zone id already exists', () => {
            assert.throws(() => { napa.zone.create('napa-zone1'); });
        });

        it('@napa: zone id already exists', () => {
            return shouldFail(() => {
                return napaZone1.execute(`${napaLibPath}/zone`, 'create', ['napa-zone1']);
            });
        });
    });

    describe("get", () => {
        it('@node: get node zone', () => {
            let zone = napa.zone.get('node');
            assert(zone != null);
            assert.strictEqual(zone.id, 'node');
        });

        it('@node: get napa zone', () => {
            let zone = napa.zone.get('napa-zone1');
            assert(zone != null);
            assert.strictEqual(zone.id, 'napa-zone1');
        });

        it('@napa: get napa zone', async () => {
            let result = await napaZone1.execute(`${napaLibPath}/zone`, "get", ['napa-zone1']);
            assert.strictEqual(result.value.id, 'napa-zone1');
        });

        it('@napa: get node zone', async () => {
            let result = await napaZone1.execute(`${napaLibPath}/zone`, "get", ['node']);
            assert.strictEqual(result.value.id, 'node');
        });

        it('@node: get napa created zone', () => {
            let zone = napa.zone.get('napa-zone2');
            assert(zone != null);
            assert.strictEqual(zone.id, 'napa-zone2');
        });

        it('@napa: get napa created zone', async () => {
            let result = await napaZone1.execute(`${napaLibPath}/zone`, 'get', ['napa-zone2']);
            assert.strictEqual(result.value.id, 'napa-zone2');
        });

        it('@node: id not existed', () => {
            assert.throws(() => { napa.zone.get('zonex'); });
        });

        it('@napa: zone not existed', () => {
            return shouldFail(() => {
                return napaZone1.execute(`${napaLibPath}/zone`, 'get', ['zonex']); 
            });
        });
    });

    describe("currentZone", () => {
        it('@node', () => {
            assert.strictEqual(napa.zone.current.id, 'node');
        });

        it('@napa', async () => {
            let result = await napaZone1.execute('./napa-zone/test', "getCurrentZone");
            assert.strictEqual(result.value.id, 'napa-zone1');
        });
    });

    describe('broadcast', () => {
        it('@node: -> node zone with JavaScript code', () => {
            return napa.zone.current.broadcast("var state = 0;");
        });

        it('@node: -> napa zone with JavaScript code', () => {
            return napaZone1.broadcast("var state = 0;");
        });

        it('@napa: -> napa zone with JavaScript code', () => {
            return napaZone1.execute('./napa-zone/test', "broadcast", ["napa-zone2", "var state = 0;"]);
        });

        it('@napa: -> napa zone with JavaScript code', () => {
            return napaZone1.execute('./napa-zone/test', "broadcast", ["napa-zone1", "var state = 0;"]);
        });

        it('@napa: -> node zone with JavaScript code', () => {
            return napaZone1.execute('./napa-zone/test', "broadcast", ["node", "var state = 0;"]);
        });

        it('@node: bad JavaScript code', () => {
            return shouldFail(() => {
                return napaZone1.broadcast("var state() = 0;");
            });
        });

        it('@napa: bad JavaScript code', () => {
            return shouldFail(() => {
                return napaZone1.execute('./napa-zone/test', "broadcast", ["napa-zone2", "var state() = 0;"]);
            });
        });

        it('@node: -> node zone throw runtime error', () => {
            return shouldFail(() => {
                return napa.zone.current.broadcast("throw new Error();");
            });
        });

        it('@node: -> napa zone throw runtime error', () => {
            return shouldFail(() => {
                return napaZone1.broadcast("throw new Error();");
            });
        });

        it('@napa: -> napa zone throw runtime error', () => {
            return shouldFail(() => {
                return napaZone1.execute('./napa-zone/test', "broadcast", ["napa-zone2", "throw new Error();"]);
            });
        });

        it('@napa: -> node zone throw runtime error', () => {
            return shouldFail(() => {
                return napaZone1.execute('./napa-zone/test', "broadcast", ["node", "throw new Error();"]);
            });
        });

        it('@node: -> node zone with anonymous function', () => {
            return napa.zone.current.broadcast((input: string) => {
                console.log(input);
            }, ['hello world']);
        });

        it('@node: -> napa zone with anonymous function', () => {
            return napaZone1.broadcast((input: string) => {
                console.log(input);
            }, ['hello world']);
        });

        it('@napa: -> napa zone with anonymous function', () => {
            return napaZone1.execute('./napa-zone/test', "broadcastTestFunction", ['napa-zone2']);
        });

        it('@napa: -> node zone with anonymous function', () => {
            return napaZone1.execute('./napa-zone/test', "broadcastTestFunction", ['node']);
        });

        // TODO #4: support transportable args in broadcast.
        it.skip('@node: -> node zone with transportable args', () => {
            return napa.zone.current.broadcast((allocator: any) => {
                console.log(allocator);
            }, [napa.memory.crtAllocator]);
        });

        /// TODO #4: support transportable tags in broadcast.
        it.skip('@node: -> napa zone with transportable args', () => {
            return napaZone1.broadcast((allocator: any) => {
                console.log(allocator);
            }, [napa.memory.crtAllocator]);
        });

        // Blocked by TODO #4.
        it.skip('@napa: -> napa zone with transportable args', () => {
            return napaZone1.execute('./napa-zone/test', "broadcastTransportable", ['napa-zone2']);
        });

        // Blocked by TODO #4.
        it.skip('@napa: -> node zone with transportable args', () => {
            return napaZone1.execute('./napa-zone/test', "broadcastTransportable", ['node']);
        });

        it('@node: -> node zone with anonymous function having closure (should fail)', () => {
            return shouldFail(() => {
                return napa.zone.current.broadcast(() => {
                    console.log(napaZone1.id);
                });
            });
        });

        it('@node: -> napa zone with anonymous function having closure (should fail)', () => {
            return shouldFail(() => {
                return napaZone1.broadcast(() => {
                    console.log(napaZone1.id);
                });
            });
        });

        it('@napa: -> napa zone with anonymous function having closure (should fail)', () => {
            return shouldFail(() => {
                return napaZone1.execute('./napa-zone/test', "broadcastClosure", ['napa-zone2']);
            });
        });

        it('@napa: -> node zone with anonymous function having closure (should fail)', () => {
            return shouldFail(() => {
                return napaZone1.execute('./napa-zone/test', "broadcastClosure", ['node']);
            });
        });
    });

    describe('execute', () => {
        let fooDef = 'function foo(input) { return input; }';
        let nestedFunctionDef = `
            var ns1 = {
                ns2: {
                    foo: function (input) {
                    return input;
                    }
                }
            };
        `;

        napaZone1.broadcast(fooDef);
        napaZone1.broadcast(nestedFunctionDef);
        napaZone2.broadcast(fooDef);
        
        // TODO::Seems this line is not executed correctly here.
        // The weird thing is that the same logic can be executed correctly in other place,
        // like in broadcast section. 
        // This leads to the below 2 test cases failed.
        //     1. @node: -> node zone with global function name
        //     2. @napa: -> node zone with global function name
        napa.zone.node.broadcast(fooDef);
        napa.zone.node.broadcast(nestedFunctionDef);

        it('@node: -> node zone with global function name', () => {
            return napa.zone.current.execute("", "foo", ['hello world'])
                .then((result: napa.zone.Result) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        it('@node: -> napa zone with global function name', () => {
            return napaZone1.execute("", "foo", ['hello world'])
                .then((result: napa.zone.Result) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        it('@napa: -> napa zone with global function name', () => {
            return napaZone1.execute('./napa-zone/test', 'execute', ["napa-zone2", "", "foo", ['hello world']])
                .then((result: napa.zone.Result) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        it('@napa: -> node zone with global function name', () => {
            return napaZone1.execute('./napa-zone/test', 'execute', ["node", "", "foo", ['hello world']])
                .then((result: napa.zone.Result) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        it('@node: -> napa zone with global function name: function with namespaces', () => {
            return napaZone1.execute("", "ns1.ns2.foo", ['hello world'])
                .then((result: napa.zone.Result) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        it('@node: -> node zone with global function name not exists', () => {
            return shouldFail(() => {
                return napa.zone.current.execute("", "foo1", ['hello world']);
            });
        });

        it('@node: -> napa zone with global function name not exists', () => {
            return shouldFail(() => {
                return napaZone1.execute("", "foo1", ['hello world']);
            });
        });

        it('@napa: -> napa zone with global function name not exists', () => {
            return shouldFail(() => {
                return napaZone1.execute('./napa-zone/test', 'execute', ["napa-zone2", "", "foo1", []]);
            });
        });

        it('@napa: -> node zone with global function name not exists', () => {
            return shouldFail(() => {
                return napaZone1.execute('./napa-zone/test', 'execute', ["node", "", "foo1", []]);
            });
        });

        it('@node: -> node zone with module function name', () => {
            return napa.zone.current.execute('./napa-zone/test', "bar", ['hello world'])
                .then((result: napa.zone.Result) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        it('@node: -> napa zone with module function name', () => {
            return napaZone1.execute('./napa-zone/test', "bar", ['hello world'])
                .then((result: napa.zone.Result) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        it('@napa: -> napa zone with module function name', () => {
            return napaZone1.execute('./napa-zone/test', 'execute', ["napa-zone2", path.resolve(__dirname, './napa-zone/test'), "bar", ['hello world']])
                .then((result: napa.zone.Result) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        it('@napa: -> node zone with module function name', () => {
            return napaZone1.execute('./napa-zone/test', 'execute', ["node", path.resolve(__dirname, './napa-zone/test'), "bar", ['hello world']])
                .then((result: napa.zone.Result) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        it('@node: -> napa zone with module function name: function with namespaces', () => {
            return napaZone1.execute('./napa-zone/test', "ns1.ns2.foo", ['hello world'])
                .then((result: napa.zone.Result) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        it('@node: -> napa zone with module function name: module is a function', () => {
            return napaZone1.execute(path.resolve(__dirname, "./napa-zone/function-as-module"), "", ['hello world'])
                .then((result: napa.zone.Result) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        it('@node: -> node zone with module not exists', () => {
            return shouldFail(() => {
                return napa.zone.current.execute("abc", "foo1", ['hello world']);
            });
        });

        it('@node: -> napa zone with module not exists', () => {
            return shouldFail(() => {
                return napaZone1.execute("abc", "foo1", ['hello world']);
            });
        });

        it('@napa: -> napa zone with module not exists', () => {
            return shouldFail(() => {
                return napaZone1.execute('./napa-zone/test', 'execute', ["napa-zone2", "abc", ".foo", []]);
            });
        });

        it('@napa: -> node zone with module not exists', () => {
            return shouldFail(() => {
                return napaZone1.execute('./napa-zone/test', 'execute', ["node", "abc", "foo.", []]);
            });
        });

        it('@node: -> node zone with module function not exists', () => {
            return shouldFail(() => {
                return napa.zone.current.execute('./napa-zone/test', "foo1", ['hello world']);
            });
        });

        it('@node: -> napa zone with module function not exists', () => {
            return shouldFail(() => {
                return napaZone1.execute('./napa-zone/test', "foo1", ['hello world'])
            });
        });

        it('@napa: -> napa zone with module function not exists', () => {
            return shouldFail(() => {
                return napaZone1.execute('./napa-zone/test', 'execute', ["napa-zone1", './napa-zone/test', "foo1", []]);
            });
        });

        it('@napa: -> node zone with module function not exists', () => {
            return shouldFail(() => {
                return napaZone1.execute('./napa-zone/test', 'execute', ["node", './napa-zone/test', "foo1", []]);
            });
        });

        it('@node: -> node zone with anonymous function', () => {
            return napa.zone.current.execute((input: string) => {
                return input;
            }, ['hello world'])
                .then((result: napa.zone.Result) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        it('@node: -> napa zone with anonymous function', () => {
            return napaZone1.execute((input: string) => {
                return input;
            }, ['hello world'])
                .then((result: napa.zone.Result) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        it('@napa: -> napa zone with anonymous function', () => {
            return napaZone1.execute('./napa-zone/test', 'executeTestFunction', ["napa-zone2"])
                .then((result: napa.zone.Result) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        it('@napa: -> node zone with anonymous function', () => {
            return napaZone1.execute('./napa-zone/test', 'executeTestFunction', ["node"])
                .then((result: napa.zone.Result) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        it('@node: -> node zone with anonymous function having closure (should success)', () => {
            return napa.zone.current.execute(() => { return napaZone1; });
        });

        it('@node: -> napa zone with anonymous function having closure (should fail)', () => {
            return shouldFail(() => {
                return napaZone1.execute(() => { return napaZone1; });
            });
        });

        it('@napa: -> napa zone with anonymous function having closure (should fail)', () => {
            return shouldFail(() => {
                return napaZone1.execute('./napa-zone/test', 'executeTestFunctionWithClosure', ["napa-zone2"]);
            });
        });

        it('@napa: -> node zone with anonymous function having closure (should fail)', () => {
            return shouldFail(() => {
                return napaZone1.execute('./napa-zone/test', 'executeTestFunctionWithClosure', ["node"]);
            });
        });

        it('@node: -> node zone with transportable args', () => {
            return napa.zone.current.execute((allocator: napa.memory.Allocator) => {
                var assert = require("assert");
                assert.deepEqual(allocator.handle, (<any>global).napa.memory.crtAllocator.handle);
            }, [napa.memory.crtAllocator]);
        });

        it('@node: -> napa zone with transportable args', () => {
            return napaZone1.execute((allocator: napa.memory.Allocator) => {
                var assert = require("assert");
                assert.deepEqual(allocator.handle, (<any>global).napa.memory.crtAllocator.handle);
            }, [napa.memory.crtAllocator]);
        });

        it('@napa: -> napa zone with transportable args', () => {
            return napaZone1.execute('./napa-zone/test', "executeWithTransportableArgs", ['napa-zone2']);
        });

        it('@napa: -> node zone with transportable args', () => {
            return napaZone1.execute('./napa-zone/test', "executeWithTransportableArgs", ['node']);
        });

        it('@node: -> node zone with transportable returns', () => {
            return napa.zone.current.execute((allocator: napa.memory.Allocator) => {
                return allocator;
            }, [napa.memory.crtAllocator])
            .then((result: napa.zone.Result) => {
                assert.deepEqual(result.value.handle, napa.memory.crtAllocator.handle);
            });
        });

        it('@node: -> napa zone with transportable returns', () => {
            return napaZone1.execute((allocator: napa.memory.Allocator) => {
                return allocator;
            }, [napa.memory.crtAllocator])
            .then((result: napa.zone.Result) => {
                assert.deepEqual(result.value.handle, napa.memory.crtAllocator.handle);
            });
        });

        it('@napa: -> napa zone with transportable returns', () => {
            return napaZone1.execute('./napa-zone/test', "executeWithTransportableReturns", ['napa-zone2'])
                .then((result: napa.zone.Result) => {
                    assert.deepEqual(result.value.handle, napa.memory.crtAllocator.handle);
                });
        });

        it('@napa: -> node zone with transportable returns', () => {
            return napaZone1.execute('./napa-zone/test', "executeWithTransportableReturns", ['node'])
                .then((result: napa.zone.Result) => {
                    assert.deepEqual(result.value.handle, napa.memory.crtAllocator.handle);
                });
        });

        let unicodeStr = "中文 español deutsch English हिन्दी العربية português বাংলা русский 日本語 ਪੰਜਾਬੀ 한국어 தமிழ் עברית"; // len = 92

        it('@node: -> node zone with args containing unicode string', () => {
            return napa.zone.current.execute((str: string) => {
                var assert = require("assert");
                let unicodeStr = "中文 español deutsch English हिन्दी العربية português বাংলা русский 日本語 ਪੰਜਾਬੀ 한국어 தமிழ் עברית";
                assert.equal(str, unicodeStr);
                return str;
            }, [unicodeStr]).then((result: napa.zone.Result) => {
                assert.equal(result.value, unicodeStr);
            });
        });

        it('@node: -> napa zone with args containing unicode string', () => {
            return napaZone1.execute((str: string) => {
                var assert = require("assert");
                let unicodeStr = "中文 español deutsch English हिन्दी العربية português বাংলা русский 日本語 ਪੰਜਾਬੀ 한국어 தமிழ் עברית";
                assert.equal(str, unicodeStr);
                return unicodeStr;
            }, [unicodeStr]).then((result: napa.zone.Result) => {
                assert.equal(result.value, unicodeStr);
            });
        });

        it('@napa: -> napa zone with args containing unicode string', () => {
            return napaZone1.execute('./napa-zone/test', "executeWithArgsContainingUnicodeString", ['napa-zone2'])
                .then((result: napa.zone.Result) => {
                    assert.equal(result.value, unicodeStr);
                });
        });

        it('@napa: -> node zone with args containing unicode string', () => {
            return napaZone1.execute('./napa-zone/test', "executeWithArgsContainingUnicodeString", ['node'])
                .then((result: napa.zone.Result) => {
                    assert.equal(result.value, unicodeStr);
                });
        });

        it.skip('@node: -> napa zone with timeout and succeed', () => {
            return napaZone1.execute('./napa-zone/test', 'waitMS', [1], {timeout: 100});
        });
        
        it.skip('@napa: -> napa zone with timeout and succeed', () => {
            return napaZone1.execute('./napa-zone/test', 'executeTestFunctionWithTimeout', ["napa-zone2", 1], {timeout: 100});
        });

        it.skip('@node: -> napa zone with timed out in JavaScript', () => {
            return shouldFail(() => {
                return napaZone1.execute('./napa-zone/test', 'waitMS', [100], {timeout: 1});
            });
        });

        it.skip('@napa: -> napa zone with timed out in JavaScript', () => {
            return shouldFail(() => {
                return napaZone1.execute('./napa-zone/test', 'executeTestFunctionWithTimeout', ["napa-zone2", 100], {timeout: 1});
            });
        });

        it.skip('@node: -> napa zone with timed out in add-on', () => {
        });

        it.skip('@napa: -> napa zone with timed out in add-on', () => {
        });

        it.skip('@node: -> napa zone with timed out in multiple hops', () => {
        });

        it.skip('@napa: -> napa zone with timed out in multiple hops', () => {
        });
    });
});