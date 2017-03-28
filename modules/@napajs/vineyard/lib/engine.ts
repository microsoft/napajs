import * as napa from 'napajs';
import * as path from 'path';
import * as assert from 'assert';

import { Settings, Application, RequestContext} from './app';
import * as objectContext from './object-context';
import * as wire from './wire';
import * as config from './config';
import * as utils from './utils';

/// <summary> Engine config, which is the root config of @napajs/vineyard. </summary>
export interface EngineSettings extends Settings{
    /// <summary> Throw exception on error, or return Response with error code. Default is true. </summary>
    throwExceptionOnError: boolean;
}

/// <summary> Interface for application engine. </summary>
export interface Engine {
    /// <summary> Register an application instance in current engine. </summary>
    /// <param name="appModuleName"> module name of a vineyard application.</param>
    /// <param name="appInstanceNames"> a list of strings used as names of application instances.</param>
    /// <param name="container"> container to run the app. If null, use current isolate. </param>
    register(appModuleName: string, appInstanceNames: string[], container: napa.Container): void;

    /// <summary> Serve a request. </summary>
    /// <param name="request"> A JSON string or a request object. </param>
    serve(request: string | wire.Request): Promise<wire.Response>;

    /// <summary> Get application instance names served by this engine. </param>
    applicationInstanceNames: string[];
}

/// <summary> Engine on local container or Node.JS isolate. </summary>
export class LocalEngine implements Engine{
    // Lower-case name to application map.
    private _applications: Map<string, Application> = new Map<string, Application>();

    // Enabled application names. 
    private _applicationInstanceNames: string[] = [];

    // Engine settings.
    private _settings: EngineSettings;

    // Global scope object context.
    private _objectContext: objectContext.ScopedObjectContext;

    /// <summary> Constructor. </summary>
    /// <param> vinyard engine settings. </summary>
    public constructor(settings: EngineSettings = null) {
        this._settings = settings;
        this._objectContext = new objectContext.ScopedObjectContext(
            "global",
            this._settings.baseDir,
            null,
            settings.objectContext
        );
    }

    /// <summary> Register an application instance in current engine. </summary>
    /// <param name="appModuleName"> module name of a vineyard application.</param>
    /// <param name="appInstanceNames"> a list of strings used as names of application instances.</param>
    /// <param name="container"> container to run the app. If null, use current isolate. </param>
    public register(
        appModuleName: string, 
        appInstanceNames: string[], 
        container: napa.Container = null): void {

        if (container != null) {
            throw new Error("LocalEngine doesn't support register on a remote container.");
        }

        // Load application.
        let appConfigPath = require.resolve(appModuleName + '/app.json');
        let app = new Application(
                this._objectContext,
                config.ApplicationConfig.fromConfig(
                this._settings,
                appConfigPath));

        // Put application in registry.
        for (let instanceName of appInstanceNames) {
            let lowerCaseName = instanceName.toLowerCase();
            if (this._applications.has(lowerCaseName)) {
                throw new Error("Already registered with application name: '`$applicationName`'.");
            }

            this._applications.set(lowerCaseName, app);
            this._applicationInstanceNames.push(instanceName);
        }
    }

    /// <summary> Serve a request. </summary>
    /// <param name="request"> A JSON string or a request object. </param>
    public serve(request: string | wire.Request): Promise<wire.Response> {
        return new Promise<RequestContext>(resolve => {
            if (typeof request === 'string') {
                request = utils.appendMessageOnException(
                    ". Fail to parse request string.",
                    () => { return JSON.parse(<string>request);});
            }
            
            let appName = (<wire.Request>request).application;
            if (appName == null) {
                throw new Error("Property 'application' is missing from request.");
            }

            resolve(new RequestContext(
                this.getApplication(appName), 
                <wire.Request>request));
        }).then((context: RequestContext) => {
            return context.execute();
        });
    }

    /// <summary> Get application names. </summary>
    public get applicationInstanceNames(): string[] {
        return this._applicationInstanceNames;
    }

    /// <summary> Get engine level object context. </summary>
    public get objectContext(): objectContext.ScopedObjectContext {
        return this._objectContext;
    }

    /// <summary> Get application by name. </summary>
    public getApplication(appName: string): Application {
        let loweredName = appName.toLowerCase();
        if (this._applications.has(loweredName)) {
            return this._applications.get(loweredName);
        }
        throw new Error("'" + appName + "' is not a known application");
    }

    /// <summary> Get global settings. </summary>
    public get settings(): Settings {
        return this._settings;
    }
}

/// <summary> Engine on a remote NapaJS container. </summary>
export class RemoteEngine {
    /// <summary> Remote container </summary>
    private _container: napa.Container;

    /// <summary> Application instance names running on this engine. </summary>
    private _applicationInstanceNames: string[] = [];

    /// <summary> Constructor. </summary>
    public constructor(container: napa.Container) {
        assert(container != null);
        this._container = container;
    }

    /// <summary> Register an application instance in current engine. </summary>
    /// <param name="appModuleName"> module name of a vineyard application.</param>
    /// <param name="appInstanceNames"> a list of strings used as names of application instances.</param>
    /// <param name="container"> container to run the app. If null, use current isolate. </param>
    public register(appModuleName: string, 
        appInstanceNames: string[], 
        container: napa.Container = undefined): void {
        if (container != null && container != this._container) {
            throw new Error("RemoteEngine cannot register applicationfor a different container.");
        }
            
        // TODO: @dapeng. implement this after introducing container.runAllSync.
        // this._container.loadFileSync(path.resolve(__dirname, "index.js"));
        // this._container.runAllSync('register', [appModuleName, JSON.stringify(appInstanceNames)]);
        // this._applicationInstanceNames = this._applicationInstanceNames.concat(appInstanceNames);
    }

    /// <summary> Serve a request. </summary>
    /// <param name="request"> A JSON string or a request object. </param>
    public async serve(request: string | wire.Request): Promise<wire.Response> {
        //let responseString = this._container.run('serve', [JSON.stringify(request)], );
        let container = this._container;
        return new Promise<napa.Response>(resolve => {
            container.run('serve', [JSON.stringify(request)], 
                (response: napa.Response) => {
                    resolve(response);
                })
            }).then((napaResponse: napa.Response) => {
                return Promise.resolve(wire.ResponseHelper.parse(napaResponse.returnValue));
            });
    }

    /// <summary> Get application instance names served by this engine. </param>
    public get applicationInstanceNames(): string[] {
        return this._applicationInstanceNames;
    }
}

/// <summary> Engine hub. (this can only exist in Node.JS isolate) </summary>
export class EngineHub implements Engine {
    /// <summary> Local engine. Only instantiate when application is registered locally. </summary>
    private _localEngine: Engine;

    /// <summary> Container to remote engine map. </summary>
    private _remoteEngines: Map<napa.Container, Engine> = new Map<napa.Container, Engine>();

    /// <summary> Settings for local engine if needed. </summary>
    private _settings: EngineSettings;

    /// <summary> Application instance names. </summary>
    private _applicationInstanceNames: string[] = [];

    /// <summary> Application instance name to engine map. </summary>
    private _engineMap: Map<string, Engine> = new Map<string, Engine>();

    /// <summary> Constructor. </summary>
    /// <param> vinyard engine settings. </summary>
    public constructor(settings: EngineSettings = null) {
        this._settings = settings;
    }

    /// <summary> Register an application for serving. </summary>
    /// <param name="moduleName"> module name of a vineyard application.</param>
    /// <param name="applicationNames"> a list of strings used as application names</param>
    /// <param name="container"> container to run the app. If null, use current isolate. </param>
    public register(appModuleName: string, appInstanceNames: string[], container: napa.Container = null) {
        let engine: Engine = undefined;
        if (container == null) {
            if (this._localEngine == null) {
                this._localEngine = new LocalEngine(this._settings);
            }
            engine = this._localEngine;
        }
        else {
            if (this._remoteEngines.has(container)) {
                engine = this._remoteEngines.get(container);
            }
            else {
                engine = new RemoteEngine(container);
                this._remoteEngines.set(container, engine);
            }
        }
        engine.register(appModuleName, appInstanceNames, undefined);
        this._applicationInstanceNames = this._applicationInstanceNames.concat(appInstanceNames);
        for (let instanceName of this._applicationInstanceNames) {
            let lowerCaseName = instanceName.toLowerCase();
            this._engineMap.set(lowerCaseName, engine);
        }
    }

    /// <summary> Serve vineyard request. </summary>
    public async serve(request: string | wire.Request): Promise<wire.Response> {
        return new Promise<Engine>(resolve => {
            if (typeof request === 'string') {
                request = utils.appendMessageOnException(
                    ". Fail to parse request string.",
                    () => { return JSON.parse(<string>request);});
            }

            // TODO: @dapeng, avoid extra parsing/serialization for remote engine.
            let appName = (<wire.Request>request).application;
            if (appName == null) {
                throw new Error("Property 'application' is missing from request.");
            }

            let lowerCaseName = appName.toLowerCase();
            if (!this._engineMap.has(lowerCaseName)) {
                throw new Error("Application '" + appName + "' is not registered for serving");
            }
            
            resolve(this._engineMap.get(lowerCaseName));
        }).then((engine: Engine) => {
            return engine.serve(request);
        });
    }

    /// <summary> Get application instance names. </summary>
    public get applicationInstanceNames(): string[] {
        return this._applicationInstanceNames;
    }
}

