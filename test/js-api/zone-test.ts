import * as napa from "napajs";
import * as assert from "assert";
import * as path from "path";
 
napa.setPlatformSettings({ loggingProvider: "nop" });

describe('napajs/zone', () => {
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

        /// Blocked by bug #2
        it('@napa', () => {
            let result = napaZone1.executeSync('napajs', "getCurrentZone", []);
            assert.strictEqual(result.value.id, 'napa-zone1');
        });
    });

    describe('broadcast', () => {
        // TODO #1: implement NodeZone.
        it.skip('@node: -> node zone with JavaScript code', (done : (error?: any) => void) => {
            /// TODO: change broadcast response code to void and convert non-success to exceptions.
            napa.getCurrentZone().broadcast("var state = 0;")
                .then((code: number) => {
                    done();
                })
                .catch((error) => {
                    done(error);
                });
        });

        // TODO #2: change return value to Promise<void> and use exception/reject to replicate error.
        // Bug #3: promise.then is always fired after mocha test timeout.
        it.skip('@node: -> napa zone with JavaScript code', (done : (error?: any) => void) => {
            napaZone1.broadcast("var state = 0;")
                .then((code: number) => {
                    done();
                })
                .catch((error) => {
                    done(error);
                });
        });

        // TODO #3: better support async target function in broadcast/execute.
        // Bug #4: zone.broadcast in napa zone will hang forever.
        it.skip('@napa: -> napa zone with JavaScript code', () => {
            let succeed = false;
            try {
                /// TODO: emit exception on executeSync if execution failed.
                let result = napaZone1.executeSync(napaZoneTestModule, "broadcast", ["napa-zone1", "var state = 0;"]);
                succeed = true;
            }
            catch (err) {
                console.log(err);
            }
            finally {
                assert(succeed, "should succeed");
            }
        });

        // Blocked by TODO #1.
        it.skip('@napa: -> node zone with JavaScript code', (done : (error?: any) => void) => {
            /// TODO: emit exception on executeSync if execution failed.
            napaZone1.executeSync(napaZoneTestModule, "broadcast", ["node", "var state = 0;"]);
        });

        // Blocked by Bug #3
        it.skip('@node: bad JavaScript code', (done : () => void) => {
            napaZone1.broadcast("var state() = 0;")
                .then(() => {
                    assert(false, "Should not succeed");
                })
                .catch((error: any) => {
                    done();
                });
        });

        // Bug #5: Empty MaybeLocal, scheduler/worker.cpp: 142.
        it.skip('@napa: bad JavaScript code', (done : () => void) => {
            napaZone1.executeSync(napaZoneTestModule, "broadcast", ["napa-zone2", "var state() = 0;"]);
        });

        // Blocked by TODO #1.
        it.skip('@node: -> node zone throw runtime error', (done : (error?: any) => void) => {
            napa.getCurrentZone().broadcast("throw new Error();")
                .then(() => {
                    done("Should fail");
                })
                .catch((error: any) => {
                    done();
                });
        });
       
        // Blocked by Bug #3.
        it.skip('@node: -> napa zone throw runtime error', (done : (error?: any) => void) => {
            napaZone1.broadcast("throw new Error();")
                .then((code: number) => {
                    console.log("code:" + code);
                    done("should fail.");
                })
                .catch((error: any) => {
                    console.log(error);
                    done();
                });
        });
        
        // Blocked by Bug #4.
        it.skip('@napa: -> napa zone throw runtime error', () => {
            napaZone1.executeSync(napaZoneTestModule, "broadcast", ["napa-zone1", "throw new Error();"]);
        });
        
        // Blocked by TODO #1.
        it.skip('@napa: -> node zone throw runtime error', () => {
            napaZone1.executeSync(napaZoneTestModule, "broadcast", ["node", "throw new Error();"]);
        });
        
        // Blocked by TODO #1.
        it.skip('@node: -> node zone with anonymous function', (done : (error?: any) => void) => {
            napa.getCurrentZone().broadcast((input: string) => {
                    console.log(input);
                }, ['hello world'])
                .then((code: number) => {
                    done();
                })
                .catch(() => {
                    assert(false, "Should not fail.");
                    done();
                });
        });

        // Blocked by Bug #3.
        it.skip('@node: -> napa zone with anonymous function', (done : (error?: any) => void) => {
            napaZone1.broadcast((input: string) => {
                    console.log(input);
                }, ['hello world'])
                .then((code: number) => {
                    done();
                })
                .catch((error: any) => {
                    done(error);
                });
        });

        // Blocked by Bug #4.
        it.skip('@napa: -> napa zone with anonymous function', () => {
            napaZone1.executeSync(napaZoneTestModule, "broadcastTestFunction", ['napa-zone1']);
        });

        // Blocked by TODO #1.
        it.skip('@napa: -> node zone with anonymous function', (done : () => void) => {
            napaZone1.executeSync(napaZoneTestModule, "broadcastTestFunction", ['node']);
        });

        // TODO #4: support transportable args in broadcast.
        // Also blocked by TODO #1.
        it.skip('@node: -> node zone with transportable args', (done : () => void) => {
            napa.getCurrentZone().broadcast((allocator: any) => {
                    console.log(allocator);
                }, [napa.memory.crtAllocator])
                .then((code: number) => {
                    done();
                })
                .catch(() => {
                    assert(false, "Should not fail.");
                    done();
                });
        });

        /// TODO #4: support transportable tags in broadcast.
        it.skip('@node: -> napa zone with transportable args', (done : () => void) => {
            napaZone1.broadcast((allocator: any) => {
                    console.log(allocator);
                }, [napa.memory.crtAllocator])
                .then((code: number) => {
                    done();
                })
                .catch(() => {
                    assert(false, "Should not fail.");
                    done();
                });
        });

        // Blocked by TODO #4.
        it.skip('@napa: -> napa zone with transportable args', (done : () => void) => {
            napaZone1.executeSync(napaZoneTestModule, "broadcastTransportable", []);
        });

        // Blocked by TODO #4.
        it.skip('@napa: -> node zone with transportable args', (done : () => void) => {
            napaZone1.executeSync(undefined, "broadcastTransportable", []);
        });

        // Blocked by TODO #1.
        it.skip('@node: -> node zone with anonymous function having closure (should fail)', (done : (error?: any) => void) => {
            napa.getCurrentZone().broadcast(() => {
                    console.log(napaZone1.id);
                }, [])
                .then((code: number) => {
                    done("should fail");
                })
                .catch((error: any) => {
                    done();
                });
        });

        /// Blocked by Bug #3.
        it.skip('@node: -> napa zone with anonymous function having closure (should fail)', (done : (error?: any) => void) => {
            napaZone1.broadcast(() => {
                    console.log(napaZone1.id);
                }, [])
                .then((code: number) => {
                    done("should fail");
                })
                .catch((error: any) => {
                    done();
                });
        });

        /// Blocked by Bug #4.
        it.skip('@napa: -> napa zone with anonymous function having closure (should fail)', (done : () => void) => {
            napaZone1.executeSync(napaZoneTestModule, "broadcastClosure", ['napa-zone1']);
        });

        /// Blocked by TODO #1.
        it.skip('@napa: -> node zone with anonymous function having closure (should fail)', (done : () => void) => {
            napaZone1.executeSync(napaZoneTestModule, "broadcastClosure", ['node']);
        });
    });

    describe("broadcastSync", () => {
        /// Blocked by TODO #1.
        it.skip('@node: -> node zone with JavaScript code', () => {
            let code = napa.getCurrentZone().broadcastSync("var state = 0;");
            assert.strictEqual(code, 0);
        });

        it('@node: -> napa zone with JavaScript code', () => {
            let code = napaZone1.broadcastSync("var state = 0;");
            assert.strictEqual(code, 0);
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
            let code = napa.getCurrentZone().broadcastSync(
                (input: string) => {
                    console.log(input);
                }, 
                ['hello world']);
            
            assert.strictEqual(code, 0);
        });

        it('@node: -> napa zone with anonymous function', () => {
            let code = napaZone1.broadcastSync(
                (input: string) => {
                    console.log(input);
                }, 
                ['hello world']);
            
            assert.strictEqual(code, 0);
        });

        // Duplicated with async broadcast versin for now.
        it.skip('@napa: -> napa zone with anonymous function', () => {
        });

        // Blocked by TODO #1
        // Duplicated with async broadcast version for now
        it.skip('@napa: -> node zone with anonymous function', () => {
        });

        // Blocked by TODO #1
        it.skip('@node: -> node zone with runtime error', () => {
            let code = napa.getCurrentZone().broadcastSync(
                () => {
                    throw new Error();
                }, 
                ['hello world']);

            assert.strictEqual(code, 0);
        });

        it('@node: -> napa zone with runtime error', () => {
            let code = napaZone1.broadcastSync(
                () => {
                    throw new Error();
                }, 
                ['hello world']);

            assert.notStrictEqual(code, 0);
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

        // Blocked by TODO #1.
        it.skip('@node: -> node zone with global function name', (done: (error?: any) => void) => {
            napa.getCurrentZone().execute("", "foo", ['hello world'])
                .then((result: napa.ExecuteResult) => {
                    if (result.value === 'hello world') {
                        done();
                    }
                    else {
                        done(`Expected: 'hello world', Actual: #{result.payload}`);
                    }
                })
                .catch(() => {
                    done("Should not throw exception");
                });
        });

        // Blocked by Bug #3.
        it.skip('@node: -> napa zone with global function name', (done: (error?: any) => void) => {
            napaZone1.execute("", "foo", ['hello world'])
                .then((result: napa.ExecuteResult) => {
                    if (result.value === 'hello world') {
                        done();
                    }
                    else {
                        done(`Expected: 'hello world', Actual: #{result.payload}`);
                    }
                })
                .catch(() => {
                    done("Should not throw exception");
                });
        });

        // Blocked by TODO #3.
        it.skip('@napa: -> napa zone with global function name', () => {
            let result = napaZone1.executeSync(napaZoneTestModule, 'execute', ["napa-zone1", "", "foo", ['hello world']]);
            assert.strictEqual(result.value, "hello world");
        });

        // Blocked by TODO #1 and #3,
        it.skip('@napa: -> node zone with global function name', () => {
            let result = napaZone1.executeSync(napaZoneTestModule, 'execute', ["node", "", "foo", ['hello world']]);
            assert.strictEqual(result.value, "hello world");
        });

        // Blocked by TODO #1.
        it.skip('@node: -> node zone with global function name not exists', (done: (error?: any) => void) => {
            napa.getCurrentZone().execute("", "foo1", ['hello world'])
                .then((result: napa.ExecuteResult) => {
                    done('should fail')
                })
                .catch(() => {
                    done();
                });
        });

        // Blocked by Bug #3.
        it.skip('@node: -> napa zone with global function name not exists', (done: (error?: any) => void) => {
            napaZone1.execute("", "foo1", ['hello world'])
                .then((result: napa.ExecuteResult) => {
                    done('should fail')
                })
                .catch(() => {
                    done();
                });
        });

        it('@napa: -> napa zone with global function name not exists', () => {
            let succeed = false;
            try {
                napaZone1.executeSync(napaZoneTestModule, 'execute', ["napa-zone1", "", "foo1", []]);
                succeed = true;
            }
            catch (error) {
            }
            assert(!succeed);
        });

        // Blocked by TODO #1.
        it.skip('@napa: -> node zone with global function name not exists', () => {
            let succeed = false;
            try {
                napaZone1.executeSync(napaZoneTestModule, 'execute', ["node", "", "foo1", []]);
                succeed = true;
            }
            catch (error) {
            }
            assert(!succeed);
        });

        // Blocked by TODO #1.
        it.skip('@node: -> node zone with module function name', (done: (error?: any) => void) => {
            napa.getCurrentZone().execute(napaZoneTestModule, "bar", ['hello world'])
                .then((result: napa.ExecuteResult) => {
                    if (result.value === 'hello world') {
                        done();
                    }
                    else {
                        done(`Expected: 'hello world', Actual: #{result.payload}`);
                    }
                })
                .catch(() => {
                    done("should not fail");
                });
        });

        // Blocked by Bug #3.
        it.skip('@node: -> napa zone with module function name', (done: (error?: any) => void) => {
            napaZone1.execute(napaZoneTestModule, "bar", ['hello world'])
                .then((result: napa.ExecuteResult) => {
                    if (result.value === 'hello world') {
                        done();
                    }
                    else {
                        done(`Expected: 'hello world', Actual: #{result.payload}`);
                    }
                })
                .catch(() => {
                    done("should not fail");
                });
        });
        
        it('@napa: -> napa zone with module function name', () => {
            let result = napaZone1.executeSync(napaZoneTestModule, 'execute', ["napa-zone1", napaZoneTestModule, "bar", ['hello world']]);
            assert.strictEqual(result.value, "hello world");
        });

        // Blocked by TODO #1.
        it.skip('@napa: -> node zone with module function name', () => {
            let result = napaZone1.executeSync(napaZoneTestModule, 'execute', ["node", napaZoneTestModule, "bar", ['hello world']]);
            assert.strictEqual(result.value, "hello world");
        });

        // Blocked by TODO #1.
        it.skip('@node: -> node zone with module not exists', (done: (error?: any) => void) => {
             napa.getCurrentZone().execute("abc", "foo1", ['hello world'])
                .then((result: napa.ExecuteResult) => {
                    done('should fail');
                })
                .catch(() => {
                    done();
                });
        });

        // Blocked by Bug #3.
        it.skip('@node: -> napa zone with module not exists', (done: (error?: any) => void) => {
            napaZone1.execute("abc", "foo1", ['hello world'])
                .then((result: napa.ExecuteResult) => {
                    done('should fail');
                })
                .catch(() => {
                    done();
                });
        });

        it('@napa: -> napa zone with module not exists', () => {
            let succeed = false;
            try {
                napaZone1.executeSync(napaZoneTestModule, 'execute', ["napa-zone1", "abc", "foo1", []]);
                succeed = true;
            }
            catch (error) {
            }
            assert(!succeed);
        });

        // Blocked by TODO #1.
        it.skip('@napa: -> node zone with module not exists', () => {
            let succeed = false;
            try {
                napaZone1.executeSync(napaZoneTestModule, 'execute', ["node", "abc", "foo1", []]);
                succeed = true;
            }
            catch (error) {
            }
            assert(!succeed);
        });

        // Blocked by TODO #1.
        it.skip('@node: -> node zone with module function not exists', (done: (error?: any) => void) => {
            napa.getCurrentZone().execute(napaZoneTestModule, "foo1", ['hello world'])
                .then((result: napa.ExecuteResult) => {
                    done('should fail');
                })
                .catch(() => {
                    done();
                });
        });

        // Blocked by Bug #3.
        it.skip('@node: -> napa zone with module function not exists', (done: (error?: any) => void) => {
             napaZone1.execute(napaZoneTestModule, "foo1", ['hello world'])
                .then((result: napa.ExecuteResult) => {
                    done('should fail');
                })
                .catch(() => {
                    done();
                });
        });

        it('@napa: -> napa zone with module function not exists', () => {
            let succeed = false;
            try {
                napaZone1.executeSync(napaZoneTestModule, 'execute', ["napa-zone1", napaZoneTestModule, "foo1", []]);
                succeed = true;
            }
            catch (error) {
            }
            assert(!succeed);
        });

        // Blocked by TODO #1.
        it.skip('@napa: -> node zone with module function not exists', () => {
            let succeed = false;
            try {
                napaZone1.executeSync(napaZoneTestModule, 'execute', ["node", napaZoneTestModule, "foo1", []]);
                succeed = true;
            }
            catch (error) {
            }
            assert(!succeed);
        });

        // Blocked by TODO #1.
        it.skip('@node: -> node zone with anonymous function', (done: (error?: any) => void) => {
            napa.getCurrentZone().execute((input: string) => {
                    return input;
                }, ['hello world'])
                .then((result: napa.ExecuteResult) => {
                    if (result.value === "hello world") {
                        done();
                    }
                    else {
                        done(`Expected: 'hello world', Actual: #{result.payload}`);
                    }
                })
                .catch(() => {
                    done();
                });
        });

        // TODO #5: implment anonymous function in execute/executeSync.
        it.skip('@node: -> napa zone with anonymous function', (done: (error?: any) => void) => {
            napaZone1.execute((input: string) => {
                    return input;
                }, ['hello world'])
                .then((result: napa.ExecuteResult) => {
                    if (result.value === "hello world") {
                        done();
                    }
                    else {
                        done(`Expected: 'hello world', Actual: #{result.payload}`);
                    }
                })
                .catch(() => {
                    done();
                });
        });

        // Blocked by TODO #5.
        it.skip('@napa: -> napa zone with anonymous function', () => {
            let result = napaZone1.executeSync(napaZoneTestModule, 'executeTestFunction', ["napa-zone1"]);
            assert.strictEqual(result.value, "hello world");
        });

        // Blocked by TODO #1.
        it.skip('@napa: -> node zone with anonymous function', () => {
            let result = napaZone1.executeSync(napaZoneTestModule, 'executeTestFunction', ["node"]);
            assert.strictEqual(result.value, "hello world");
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

        it('@node: -> napa zone with transportable args', () => {
        });

        it('@napa: -> napa zone with transportable args', () => {
        });

        // Blocked by TODO #1.
        it.skip('@napa: -> node zone with transportable args', () => {
        });

        // Blocked by TODO #1.
        it.skip('@node: -> node zone with transportable returns', () => {
        });

        it('@node: -> napa zone with transportable returns', () => {
        });

        it('@napa: -> napa zone with transportable returns', () => {
        });

        // Blocked by TODO #1.
        it.skip('@napa: -> node zone with transportable returns', () => {
        });

        /// Timeout is not available in node zone.
        it('@node: -> napa zone with timeout and succeed', () => {
        });
        
        it('@napa: -> napa zone with timeout and succeed', () => {
        });

        /// Timeout is not available in node zone.
        it('@node: -> napa zone with timed out in JavaScript', () => {
        });

        it('@napa: -> napa zone with timed out in JavaScript', () => {
        });

        it('@node: -> napa zone with timed out in add-on', () => {
        });

        it('@napa: -> napa zone with timed out in add-on', () => {
        });

        it('@node: -> napa zone with timed out in multiple hops', () => {
        });

        it('@napa: -> napa zone with timed out in multiple hops', () => {
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