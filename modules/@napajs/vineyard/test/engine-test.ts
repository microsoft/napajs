import * as app from '../lib/app';
import * as config from "../lib/config";
import * as builtins from '../lib/builtins';
import * as wire from '../lib/wire';
import { Engine, LocalEngine, RemoteEngine, EngineHub } from '../lib/engine';

import * as path from 'path';
import * as napa from 'napajs';
import * as assert from 'assert';

describe('@napajs/vineyard/engine', () => {
    describe('LocalEngine', () => {
        napa.initialize();
        let engine: LocalEngine = undefined;
        it('#ctor', () => {
            engine = new LocalEngine(
                config.EngineConfig.fromConfig(
                    require.resolve('../config/engine.json'))
            );
        });

        it('#register: success', () => {
            engine.register(
                path.resolve(__dirname, './test-app'), 
                ["testApp"]);
        });

        it('#register: fail - duplicated instance name', () => {
            assert.throws(() => {
                engine.register(
                    path.resolve(__dirname, './test-app'),
                    ["testApp"]);
            });
        });
            
        it('#register: fail - register for another container.', () => {
            assert.throws(() => {
                engine.register(
                    path.resolve(__dirname, './test-app'), 
                    ["testApp"], 
                    napa.createContainer());
            });
        });

        it('#serve: sync entrypoint', (done) => {
            engine.serve({
                application: "testApp",
                entryPoint: "foo",
                input: "hello world"
            }).then((response: wire.Response) => {
                assert.equal(response.responseCode, wire.ResponseCode.Success);
                assert.equal(response.output, 'hello world');
                done();
            });
        });
        
        it('#serve: async entrypoint', (done) => {
            engine.serve({
                application: "testApp",
                entryPoint: "bar",
                input: "hello world"
            }).then((response: wire.Response) => {
                assert.equal(response.responseCode, wire.ResponseCode.Success);
                assert.equal(response.output, "hello world");
                done();
            })
        });

        it('#serve: bad request - malformat JSON ', (done) => {
            engine.serve(`{
                "application": "testApp",
                "entryPoint": "foo",
            }`).catch((error: Error) => {
                done(error.message === "Unexpected token }. Fail to parse request string." 
                    ? undefined : error);
            });
        });
        
        it('#serve: bad request - not registered application ', (done) => {
            engine.serve({
                application: "testApp2",
                entryPoint: "foo"
            }).catch((error: Error) => {
                done(error.message === "'testApp2' is not a known application"? undefined: error);
            });
        });

        it('#serve: bad request - entryPoint not found ', (done) => {
            engine.serve({
                application: "testApp",
                entryPoint: "foo2"
            }).catch((error: Error) => {
                done(error.message === "Entrypoint does not exist: 'foo2'" ? undefined: error);
            });
        });

        it('#serve: application throws exception ', (done) => {
            engine.serve({
                application: "testApp",
                entryPoint: "alwaysThrow"
            }).catch((error) => {
                done(error.message === "You hit an always-throw entrypoint."? undefined: error);
            });
        });

        it('#applicationInstanceNames', () => {
            assert.deepEqual(engine.applicationInstanceNames, ["testApp"]);
        });
    });

    describe.skip('RemoteEngine', () => {
        let engine: RemoteEngine = undefined;
        let container: napa.Container = napa.createContainer();
        it('#ctor', () => {
            engine = new RemoteEngine(container);
        });

        it('#register: success', () => {
            engine.register(
                path.resolve(__dirname, './test-app'), 
                ["testApp"]);
        });

        it('#register: fail - duplicated instance name', () => {
            assert.throws(() => {
                engine.register(
                    path.resolve(__dirname, './test-app'),
                    ["testApp"]);
            });
        });
            
        it('#register: fail - register for another container.', () => {
            assert.throws(() => {
                engine.register(
                    path.resolve(__dirname, './test-app'), 
                    ["testApp"], 
                    napa.createContainer());
            });
        });

        it('#serve: sync entrypoint', (done) => {
            engine.serve({
                application: "testApp",
                entryPoint: "foo",
                input: "hello world"
            }).then((response: wire.Response) => {
                assert.equal(response.responseCode, wire.ResponseCode.Success);
                assert.equal(response.output, 'hello world');
                done();
            });
        });
        
        it('#serve: async entrypoint', (done) => {
            engine.serve({
                application: "testApp",
                entryPoint: "bar",
                input: "hello world"
            }).then((response: wire.Response) => {
                assert.equal(response.responseCode, wire.ResponseCode.Success);
                assert.equal(response.output, "hello world");
                done();
            })
        });

        it('#serve: bad request - malformat JSON ', (done) => {
            engine.serve(`{
                "application": "testApp",
                "entryPoint": "foo",
            }`).catch((error: Error) => {
                done(error.message === "Unexpected token }. Fail to parse request string." 
                    ? undefined : error);
            });
        });
        
        it('#serve: bad request - not registered application ', (done) => {
            engine.serve({
                application: "testApp2",
                entryPoint: "foo"
            }).catch((error: Error) => {
                done(error.message === "'testApp2' is not a known application"? undefined: error);
            });
        });

        it('#serve: bad request - entryPoint not found ', (done) => {
            engine.serve({
                application: "testApp",
                entryPoint: "foo2"
            }).catch((error: Error) => {
                done(error.message === "Entrypoint does not exist: 'foo2'" ? undefined: error);
            });
        });

        it('#serve: application throws exception ', (done) => {
            engine.serve({
                application: "testApp",
                entryPoint: "alwaysThrow"
            }).catch((error) => {
                done(error.message === "You hit an always-throw entrypoint."? undefined: error);
            });
        });

        it('#applicationInstanceNames', () => {
            assert.deepEqual(engine.applicationInstanceNames, ["testApp"]);
        });
    });

    // TODO: @dapeng, implement after RemoteEngine is ready.
    describe("EngineHub", () => {
        it('#register: local');
        it('#register: remote');
        it('#serve: local');
        it('#serve: remote');
    });
});