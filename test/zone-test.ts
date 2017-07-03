import * as napa from "..";
import * as assert from "assert";
import * as path from "path";

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
    // disable timeouts. 
    // promise.then is always fired after mocha test timeout.
    this.timeout(0);

    let napaZone1: Zone = napa.zone.create('napa-zone1');
    let napaZone2: Zone = napa.zone.create('napa-zone2');
    let napaZoneTestModule: string = path.resolve(__dirname, 'napa-zone/test');

    describe('create', () => {
        it('@node: default settings', () => {
            assert(napaZone1 != null);
            assert.strictEqual(napaZone1.id, 'napa-zone1');
        });

        it('@napa: default settings', async () => {
            // Zone should be destroyed when going out of scope.
            let result = await napaZone1.execute('../lib/zone', 'create', ['new-zone']);
            assert.equal(result.value.id, "new-zone");
        });

        it('@node: zone id already exists', () => {
            assert.throws(() => { napa.zone.create('napa-zone1'); });
        });

        it('@napa: zone id already exists', () => {
            shouldFail(() => {
                return napaZone1.execute('../lib/zone', 'create', ['napa-zone1']);
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
            let result = await napaZone1.execute('../lib/zone', "get", ['napa-zone1']);
            assert.strictEqual(result.value.id, 'napa-zone1');
        });

        it('@napa: get node zone', async () => {
            let result = await napaZone1.execute('../lib/zone', "get", ['node']);
            assert.strictEqual(result.value.id, 'node');
        });

        it('@node: get napa created zone', () => {
            let zone = napa.zone.get('napa-zone2');
            assert(zone != null);
            assert.strictEqual(zone.id, 'napa-zone2');
        });

        it('@napa: get napa created zone', async () => {
            let result = await napaZone1.execute('../lib/zone', 'get', ['napa-zone2']);
            assert.strictEqual(result.value.id, 'napa-zone2');
        });

        it('@node: id not existed', () => {
            assert.throws(() => { napa.zone.get('zonex'); });
        });

        it('@napa: zone not existed', () => {
            shouldFail(() => { 
                return napaZone1.execute('../lib/zone', 'get', ['zonex']); 
            });
        });
    });

    describe("currentZone", () => {
        it('@node', () => {
            assert.strictEqual(napa.zone.current.id, 'node');
        });

        it('@napa', async () => {
            let result = await napaZone1.execute(napaZoneTestModule, "getCurrentZone", []);
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
            return napaZone1.execute(napaZoneTestModule, "broadcast", ["napa-zone2", "var state = 0;"]);
        });

        // Bug #4: zone.broadcast in the same napa zone will hang forever.
        it.skip('@napa: -> napa zone with JavaScript code', () => {
            return napaZone1.execute(napaZoneTestModule, "broadcast", ["napa-zone1", "var state = 0;"]);
        });

        it('@napa: -> node zone with JavaScript code', () => {
            return napaZone1.execute(napaZoneTestModule, "broadcast", ["node", "var state = 0;"]);
        });

        it('@node: bad JavaScript code', () => {
            return shouldFail(() => {
                return napaZone1.broadcast("var state() = 0;");
            });
        });

        it('@napa: bad JavaScript code', () => {
            return shouldFail(() => {
                return napaZone1.execute(napaZoneTestModule, "broadcast", ["napa-zone2", "var state() = 0;"]);
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
                return napaZone1.execute(napaZoneTestModule, "broadcast", ["napa-zone2", "throw new Error();"]);
            });
        });

        it('@napa: -> node zone throw runtime error', () => {
            return shouldFail(() => {
                return napaZone1.execute(napaZoneTestModule, "broadcast", ["node", "throw new Error();"]);
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
            return napaZone1.execute(napaZoneTestModule, "broadcastTestFunction", ['napa-zone2']);
        });

        it('@napa: -> node zone with anonymous function', () => {
            return napaZone1.execute(napaZoneTestModule, "broadcastTestFunction", ['node']);
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
            return napaZone1.execute(napaZoneTestModule, "broadcastTransportable", []);
        });

        // Blocked by TODO #4.
        it.skip('@napa: -> node zone with transportable args', () => {
            return napa.zone.current.execute(napaZoneTestModule, "broadcastTransportable", []);
        });

        it('@node: -> node zone with anonymous function having closure (should fail)', () => {
            return shouldFail(() => {
                return napa.zone.current.broadcast(() => {
                    console.log(napaZone1.id);
                }, []);
            });
        });

        it('@node: -> napa zone with anonymous function having closure (should fail)', () => {
            return shouldFail(() => {
                return napaZone1.broadcast(() => {
                    console.log(napaZone1.id);
                }, []);
            });
        });

        it('@napa: -> napa zone with anonymous function having closure (should fail)', () => {
            return shouldFail(() => {
                return napaZone1.execute(napaZoneTestModule, "broadcastClosure", ['napa-zone2']);
            });
        });

        it('@napa: -> node zone with anonymous function having closure (should fail)', () => {
            return shouldFail(() => {
                return napaZone1.execute(napaZoneTestModule, "broadcastClosure", ['node']);
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
            return napaZone1.execute(napaZoneTestModule, 'execute', ["napa-zone2", "", "foo", ['hello world']])
                .then((result: napa.zone.Result) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        it('@napa: -> node zone with global function name', () => {
            return napaZone1.execute(napaZoneTestModule, 'execute', ["node", "", "foo", ['hello world']])
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
                return napaZone1.execute(napaZoneTestModule, 'execute', ["napa-zone2", "", "foo1", []]);
            });
        });

        it('@napa: -> node zone with global function name not exists', () => {
            return shouldFail(() => {
                return napaZone1.execute(napaZoneTestModule, 'execute', ["node", "", "foo1", []]);
            });
        });

        it('@node: -> node zone with module function name', () => {
            return napa.zone.current.execute(napaZoneTestModule, "bar", ['hello world'])
                .then((result: napa.zone.Result) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        it('@node: -> napa zone with module function name', () => {
            return napaZone1.execute(napaZoneTestModule, "bar", ['hello world'])
                .then((result: napa.zone.Result) => {
                    assert.equal(result.value, 'hello world');
                });
        });
        
        it('@napa: -> napa zone with module function name', () => {
            return napaZone1.execute(napaZoneTestModule, 'execute', ["napa-zone2", napaZoneTestModule, "bar", ['hello world']])
                .then((result: napa.zone.Result) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        it('@napa: -> node zone with module function name', () => {
            return napaZone1.execute(napaZoneTestModule, 'execute', ["node", napaZoneTestModule, "bar", ['hello world']])
                .then((result: napa.zone.Result) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        it('@node: -> napa zone with module function name: function with namespaces', () => {
            return napaZone1.execute(napaZoneTestModule, "ns1.ns2.foo", ['hello world'])
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
                return napaZone1.execute(napaZoneTestModule, 'execute', ["napa-zone2", "abc", ".foo", []]);
            });
        });

        it('@napa: -> node zone with module not exists', () => {
            return shouldFail(() => {
                return napaZone1.execute(napaZoneTestModule, 'execute', ["node", "abc", "foo.", []]);
            });
        });

        it('@node: -> node zone with module function not exists', () => {
            return shouldFail(() => {
                return napa.zone.current.execute(napaZoneTestModule, "foo1", ['hello world']);
            });
        });

        it('@node: -> napa zone with module function not exists', () => {
            return shouldFail(() => {
                return napaZone1.execute(napaZoneTestModule, "foo1", ['hello world'])
            });
        });

        it('@napa: -> napa zone with module function not exists', () => {
            return shouldFail(() => {
                return napaZone1.execute(napaZoneTestModule, 'execute', ["napa-zone1", napaZoneTestModule, "foo1", []]);
            });
        });

        it('@napa: -> node zone with module function not exists', () => {
            return shouldFail(() => {
                return napaZone1.execute(napaZoneTestModule, 'execute', ["node", napaZoneTestModule, "foo1", []]);
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
            return napaZone1.execute(napaZoneTestModule, 'executeTestFunction', ["napa-zone2"])
                .then((result: napa.zone.Result) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        it('@napa: -> node zone with anonymous function', () => {
            return napaZone1.execute(napaZoneTestModule, 'executeTestFunction', ["node"])
                .then((result: napa.zone.Result) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        it('@node: -> node zone with anonymous function having closure (should success)', () => {
            return napa.zone.current.execute(() => { return napaZone1; }, []);
        });

        it('@node: -> napa zone with anonymous function having closure (should fail)', () => {
            return shouldFail(() => {
                return napaZone1.execute(() => { return napaZone1; }, []);
            });
        });

        it('@napa: -> napa zone with anonymous function having closure (should fail)', () => {
            return shouldFail(() => {
                return napaZone1.execute(napaZoneTestModule, 'executeTestFunctionWithClosure', ["napa-zone2"]);
            });
        });

        it('@napa: -> node zone with anonymous function having closure (should fail)', () => {
            return shouldFail(() => {
                return napaZone1.execute(napaZoneTestModule, 'executeTestFunctionWithClosure', ["node"]);
            });
        });

        it.skip('@node: -> node zone with transportable args', () => {
        });

        it.skip('@node: -> napa zone with transportable args', () => {
        });

        it.skip('@napa: -> napa zone with transportable args', () => {
        });

        it.skip('@napa: -> node zone with transportable args', () => {
        });

        it.skip('@node: -> node zone with transportable returns', () => {
        });

        it.skip('@node: -> napa zone with transportable returns', () => {
        });

        it.skip('@napa: -> napa zone with transportable returns', () => {
        });

        it.skip('@napa: -> node zone with transportable returns', () => {
        });

        /// Timeout is not available in node zone.
        it.skip('@node: -> napa zone with timeout and succeed', () => {
        });
        
        it.skip('@napa: -> napa zone with timeout and succeed', () => {
        });

        /// Timeout is not available in node zone.
        it.skip('@node: -> napa zone with timed out in JavaScript', () => {
        });

        it.skip('@napa: -> napa zone with timed out in JavaScript', () => {
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