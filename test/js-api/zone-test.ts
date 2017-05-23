import * as napa from "napajs";
import * as assert from "assert";
import * as path from "path";

function shouldFail<T>(func: () => Promise<T>) {
    return func().then(
        (value: T) => {
            assert(false, "Failure was expected.");
        }, 
        (reason: any) => { 
            // Swallow the rejection since we expect failure
        });
}

napa.setPlatformSettings({ loggingProvider: "nop" });

describe('napajs/zone', function () {
    // disable timeouts. 
    // promise.then is always fired after mocha test timeout.
    this.timeout(0);

    let napaZone1: napa.Zone = napa.createZone('napa-zone1');
    let napaZone2: napa.Zone = napa.createZone('napa-zone2');
    let napaZoneTestModule: string = path.resolve(__dirname, 'napa-zone/test');

    describe('createZone', () => {
        it('@node: default settings', () => {
            assert(napaZone1 != null);
            assert.strictEqual(napaZone1.id, 'napa-zone1');
        });

        it('@napa: default settings', () => {
            // Zone should be destroyed when going out of scope.
            let result = napaZone1.executeSync('napajs', 'createZone', ['new-zone']);
            console.log(result.value);
        });

        it('@node: zone id already exists', () => {
            assert.throws(() => { napa.createZone('napa-zone1'); });
        });

        it('@napa: zone id already exists', () => {
            assert.throws(() => { napaZone1.executeSync('napajs', 'createZone', ['napa-zone1']); });
        });
    });

    describe("getZone", () => {
        it('@node: get node zone', () => {
            let zone = napa.getZone('node');
            assert(zone != null);
            assert.strictEqual(zone.id, 'node');
        });

        it('@node: get napa zone', () => {
            let zone = napa.getZone('napa-zone1');
            assert(zone != null);
            assert.strictEqual(zone.id, 'napa-zone1');
        });

        it('@napa: get napa zone', () => {
            let result = napaZone1.executeSync('napajs', "getZone", ['napa-zone1']);
            assert.strictEqual(result.value.id, 'napa-zone1');
        });

        it('@napa: get node zone', () => {
            let result = napaZone1.executeSync('napajs', "getZone", ['node']);
            assert.strictEqual(result.value.id, 'node');
        });

        it('@node: get napa created zone', () => {
            let zone = napa.getZone('napa-zone2');
            assert(zone != null);
            assert.strictEqual(zone.id, 'napa-zone2');
        });

        it('@napa: get napa created zone', () => {
            let result = napaZone1.executeSync('napajs', 'getZone', ['napa-zone2']);
            assert.strictEqual(result.value.id, 'napa-zone2');
        });

        it('@node: id not existed', () => {
            assert.throws(() => { napa.getZone('zonex'); });
        });

        it('@napa: zone not existed', () => {
            assert.throws(() => { napaZone1.executeSync('napajs', 'getZone', ['zonex']); });
        });
    });

    describe("getCurrentZone", () => {
        it('@node', () => {
            let zone = napa.getCurrentZone();
            assert.strictEqual(zone.id, 'node');
        });

        it('@napa', () => {
            let result = napaZone1.executeSync('napajs', "getCurrentZone", []);
            assert.strictEqual(result.value.id, 'napa-zone1');
        });
    });

    describe('broadcast', () => {
        // TODO #1: implement NodeZone.
        it.skip('@node: -> node zone with JavaScript code', () => {
            return napa.getCurrentZone().broadcast("var state = 0;");
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

        // Blocked by TODO #1.
        it.skip('@napa: -> node zone with JavaScript code', () => {
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

        // Blocked by TODO #1.
        it.skip('@node: -> node zone throw runtime error', () => {
            return shouldFail(() => {
                return napa.getCurrentZone().broadcast("throw new Error();");
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
        
        // Blocked by TODO #1.
        it.skip('@napa: -> node zone throw runtime error', () => {
            return shouldFail(() => {
                return napaZone1.execute(napaZoneTestModule, "broadcast", ["node", "throw new Error();"]);
            });
        });
        
        // Blocked by TODO #1.
        it.skip('@node: -> node zone with anonymous function', () => {
            return napa.getCurrentZone().broadcast((input: string) => {
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

        // Blocked by TODO #1.
        it.skip('@napa: -> node zone with anonymous function', () => {
            return napaZone1.execute(napaZoneTestModule, "broadcastTestFunction", ['node']);
        });

        // TODO #4: support transportable args in broadcast.
        // Also blocked by TODO #1.
        it.skip('@node: -> node zone with transportable args', () => {
            return napa.getCurrentZone().broadcast((allocator: any) => {
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
        // Also blocked by TODO #1.
        it.skip('@napa: -> node zone with transportable args', () => {
            return napa.getCurrentZone().execute(napaZoneTestModule, "broadcastTransportable", []);
        });

        // Blocked by TODO #1.
        it.skip('@node: -> node zone with anonymous function having closure (should fail)', () => {
            return shouldFail(() => {
                return napa.getCurrentZone().broadcast(() => {
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

        /// Blocked by TODO #1.
        it.skip('@napa: -> node zone with anonymous function having closure (should fail)', () => {
            return shouldFail(() => {
                return napaZone1.execute(napaZoneTestModule, "broadcastClosure", ['node']);
            });
        });
    });

    describe("broadcastSync", () => {
        /// Blocked by TODO #1.
        it.skip('@node: -> node zone with JavaScript code', () => {
            napa.getCurrentZone().broadcastSync("var state = 0;");
        });

        it('@node: -> napa zone with JavaScript code', () => {
            napaZone1.broadcastSync("var state = 0;");
        });

        // Duplicated with async broadcast version for now.
        it.skip('@napa: -> napa zone with JavaScript code', () => {
        });

        // Blocked by TODO #1.
        // Duplicated with async broadcast version for now.
        it.skip('@napa: -> node zone with JavaScript code', () => {            
        });

        // Blocked by TODO #1.
        it.skip('@node: -> node zone with anonymous function', () => {
            napa.getCurrentZone().broadcastSync((input: string) => {
                console.log(input);
            }, ['hello world']);
        });

        it('@node: -> napa zone with anonymous function', () => {
            napaZone1.broadcastSync((input: string) => {
                console.log(input);
            }, ['hello world']);
        });

        // Duplicated with async broadcast version for now.
        it.skip('@napa: -> napa zone with anonymous function', () => {
        });

        // Blocked by TODO #1
        // Duplicated with async broadcast version for now
        it.skip('@napa: -> node zone with anonymous function', () => {
        });

        // Blocked by TODO #1
        it.skip('@node: -> node zone with runtime error', () => {
            assert.throws(() => {
                napa.getCurrentZone().broadcastSync(() => {
                    throw new Error();
                }, ['hello world']);
            });
        });

        it('@node: -> napa zone with runtime error', () => {
            assert.throws(() => {
                napaZone1.broadcastSync(() => {
                    throw new Error();
                }, ['hello world']);
            });
        });

        // Duplicated with async broadcast version for now.
        it('@napa: -> napa zone with runtime error', () => {
        });

        // Duplicated with async broadcast version for now.
        it('@napa: -> node zone with runtime error', () => {
        });
    });

    describe('execute', () => {
        napa.getCurrentZone().broadcastSync('function foo(input) { return input; }');
        napaZone1.broadcastSync('function foo(input) { return input; }');
        napaZone2.broadcastSync('function foo(input) { return input; }');

        // Blocked by TODO #1.
        it.skip('@node: -> node zone with global function name', () => {
            return napa.getCurrentZone().execute("", "foo", ['hello world'])
                .then((result: napa.ExecuteResult) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        it('@node: -> napa zone with global function name', () => {
            return napaZone1.execute("", "foo", ['hello world'])
                .then((result: napa.ExecuteResult) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        it('@napa: -> napa zone with global function name', () => {
            return napaZone1.execute(napaZoneTestModule, 'execute', ["napa-zone2", "", "foo", ['hello world']])
                .then((result: napa.ExecuteResult) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        // Blocked by TODO #1.
        it.skip('@napa: -> node zone with global function name', () => {
            return napaZone1.execute(napaZoneTestModule, 'execute', ["node", "", "foo", ['hello world']])
                .then((result: napa.ExecuteResult) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        // Blocked by TODO #1.
        it.skip('@node: -> node zone with global function name not exists', () => {
            return shouldFail(() => {
                return napa.getCurrentZone().execute("", "foo1", ['hello world']);
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

        // Blocked by TODO #1.
        it.skip('@napa: -> node zone with global function name not exists', () => {
            return shouldFail(() => {
                return napaZone1.execute(napaZoneTestModule, 'execute', ["node", "", "foo1", []]);
            });
        });

        // Blocked by TODO #1.
        it.skip('@node: -> node zone with module function name', () => {
            return napa.getCurrentZone().execute(napaZoneTestModule, "bar", ['hello world'])
                .then((result: napa.ExecuteResult) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        it('@node: -> napa zone with module function name', () => {
            return napaZone1.execute(napaZoneTestModule, "bar", ['hello world'])
                .then((result: napa.ExecuteResult) => {
                    assert.equal(result.value, 'hello world');
                });
        });
        
        it('@napa: -> napa zone with module function name', () => {
            return napaZone1.execute(napaZoneTestModule, 'execute', ["napa-zone2", napaZoneTestModule, "bar", ['hello world']])
                .then((result: napa.ExecuteResult) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        // Blocked by TODO #1.
        it.skip('@napa: -> node zone with module function name', () => {
            return napaZone1.execute(napaZoneTestModule, 'execute', ["node", napaZoneTestModule, "bar", ['hello world']])
                .then((result: napa.ExecuteResult) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        // Blocked by TODO #1.
        it.skip('@node: -> node zone with module not exists', () => {
            return shouldFail(() => {
                return napa.getCurrentZone().execute("abc", "foo1", ['hello world']);
            });
        });

        it('@node: -> napa zone with module not exists', () => {
            return shouldFail(() => {
                return napaZone1.execute("abc", "foo1", ['hello world']);
            });
        });

        it('@napa: -> napa zone with module not exists', () => {
            return shouldFail(() => {
                return napaZone1.execute(napaZoneTestModule, 'execute', ["napa-zone2", "abc", "foo1", []]);
            });
        });

        // Blocked by TODO #1.
        it.skip('@napa: -> node zone with module not exists', () => {
            return shouldFail(() => {
                return napaZone1.execute(napaZoneTestModule, 'execute', ["node", "abc", "foo1", []]);
            });
        });

        // Blocked by TODO #1.
        it.skip('@node: -> node zone with module function not exists', () => {
            return shouldFail(() => {
                return napa.getCurrentZone().execute(napaZoneTestModule, "foo1", ['hello world']);
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

        // Blocked by TODO #1.
        it.skip('@napa: -> node zone with module function not exists', () => {
            return shouldFail(() => {
                return napaZone1.execute(napaZoneTestModule, 'execute', ["node", napaZoneTestModule, "foo1", []]);
            });
        });

        // Blocked by TODO #1 and TODO #5.
        it.skip('@node: -> node zone with anonymous function', () => {
            return napa.getCurrentZone().execute((input: string) => {
                return input;
            }, ['hello world'])
                .then((result: napa.ExecuteResult) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        // TODO #5: implment anonymous function in execute/executeSync.
        it.skip('@node: -> napa zone with anonymous function', () => {
            return napaZone1.execute((input: string) => {
                return input;
            }, ['hello world'])
                .then((result: napa.ExecuteResult) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        // Blocked by TODO #5.
        it.skip('@napa: -> napa zone with anonymous function', () => {
            return napaZone1.execute(napaZoneTestModule, 'executeTestFunction', ["napa-zone2"])
                .then((result: napa.ExecuteResult) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        // Blocked by TODO #1.
        it.skip('@napa: -> node zone with anonymous function', () => {
            return napaZone1.execute(napaZoneTestModule, 'executeTestFunction', ["node"])
                .then((result: napa.ExecuteResult) => {
                    assert.equal(result.value, 'hello world');
                });
        });

        // Blocked by TODO #1.
        it.skip('@node: -> node zone with anonymous function having closure (should fail)', () => {
        });

        it.skip('@node: -> napa zone with anonymous function having closure (should fail)', () => {
        });

        it.skip('@napa: -> napa zone with anonymous function having closure (should fail)', () => {
        });

        // Blocked by TODO #1.
        it.skip('@napa: -> node zone with anonymous function having closure (should fail)', () => {
        });

        // Blocked by TODO #1.
        it.skip('@node: -> node zone with transportable args', () => {
        });

        it.skip('@node: -> napa zone with transportable args', () => {
        });

        it.skip('@napa: -> napa zone with transportable args', () => {
        });

        // Blocked by TODO #1.
        it.skip('@napa: -> node zone with transportable args', () => {
        });

        // Blocked by TODO #1.
        it.skip('@node: -> node zone with transportable returns', () => {
        });

        it.skip('@node: -> napa zone with transportable returns', () => {
        });

        it.skip('@napa: -> napa zone with transportable returns', () => {
        });

        // Blocked by TODO #1.
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

    describe.skip("executeSync", () => {
        // Blocked by TODO #1.
        it('@node: -> node zone succeed', () => {
        });
        
        it('@node: -> napa zone succeed', () => {
        });

        it('@napa: -> napa zone succeed', () => {
        });

        // Blocked by TODO #1.
        it('@napa: -> node zone succeed', () => {
        });

        it('@node: -> node zone with runtime error', () => {
        });
        
        it('@node: -> napa zone with runtime error', () => {
        });

        it('@napa: -> napa zone with runtime error', () => {
        });

        it('@napa: -> node zone with runtime error', () => {
        });

        // Blocked by TODO #1.
        it('@node: -> node zone with runtime error', () => {
        });
        
        it('@node: -> napa zone with timed out', () => {
        });

        it('@napa: -> napa zone with timed out', () => {
        });

        // Blocked by TODO #1.
        it('@napa: -> node zone with timed out', () => {
        });
    });
});