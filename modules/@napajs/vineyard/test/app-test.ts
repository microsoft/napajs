import {Application, RequestContext} from '../lib/app';
import {LocalEngine} from '../lib/engine';

import * as config from "../lib/config";
import * as builtins from '../lib/builtins';
import * as wire from '../lib/wire';
import * as objectModel from '../lib/object-model';

import * as path from 'path';
import * as napa from 'napajs';
import * as assert from 'assert';


describe('@napajs/vineyard/app', () => {
     let engine = new LocalEngine(
            config.EngineConfig.fromConfig(
                require.resolve('../config/engine.json')));

    let app: Application = undefined; 

    describe('Application', () => {
        it('#ctor', () => {
            app = new Application(engine.objectContext,
                config.ApplicationConfig.fromConfig(
                engine.settings,
                path.resolve(__dirname, "test-app/app.json")));
        });

        it('#getters', () => {
            assert.equal(app.id, 'test-app');
            assert.equal(Object.keys(app.metrics).length, 1);
        });

        it('#create', () => {
            assert.equal(app.create({
                            _type: "TypeA",
                            value: 1
                        }), 1);
            assert.strictEqual(app.create("protocolA:/abc"), "abc");
        });

        it('#get', () => {
            assert.equal(app.get('objectA'), 1);
        });

        it('#getEntryPoint', () => {
            assert.strictEqual(app.getEntryPoint("listEntryPoints"), builtins.entryPoints.listEntryPoints);
        })
        
        it('#getInterceptor', () => {
            assert.strictEqual(app.getInterceptor("executeEntryPoint"), builtins.interceptors.executeEntryPoint);
        });

        it('#getExecutionStack', () => {
            let stack = app.getExecutionStack('foo');
            assert.equal(stack.length, 2);
            assert.strictEqual(stack[0], builtins.interceptors.finalizeResponse);
            assert.strictEqual(stack[1], builtins.interceptors.executeEntryPoint);

            stack = app.getExecutionStack('bar');
            assert.equal(stack.length, 3);
            assert.strictEqual(stack[0], builtins.interceptors.logRequestResponse);
            assert.strictEqual(stack[1], builtins.interceptors.finalizeResponse);
            assert.strictEqual(stack[2], builtins.interceptors.executeEntryPoint);
        });
    });

    describe('RequestContext', () => {
        let context: RequestContext = undefined;
        let request: wire.Request = {
            application: "testApp",
            entryPoint: "foo",
            input: "hello world"
        }

        it('#ctor', () => {
            context = new RequestContext(app, request);
        });

        it('#getters', () => {
            assert.deepEqual(context.controlFlags, {
                debug: false,
                perf: false
            });
            assert.equal(context.entryPointName, "foo");
            assert.equal(context.input, "hello world");
            assert.equal(context.traceId, "Unknown");
            assert.strictEqual(context.application, app);
            assert.strictEqual(context.entryPoint, context.getEntryPoint('foo'));
            assert.strictEqual(context.request, request);
        });

        it('#create', () => {
            assert.strictEqual(context.create({_type: "TypeA", value: 1}), 1);
        });
        
        it('#get', () => {
        });
        
        it('#getNamedObject: local only', () => {
        });
        
        it('#execute', () => {
        });

        it('#continueExecution', () => {
        });
    });

    describe('Debugger', () => {
        it('#event', () => {
        });

        it('#detail', () => {
        });

        it('#setLastError', () => {
        });

        it('#getOutput', () => {
        });
    });

    describe('RequestLogger', () => {
        it('#debug', () => {
        });

        it('#info', () => {
        });

        it('#err', () => {
        });

        it('#warn', () => {
        });
    });
});

