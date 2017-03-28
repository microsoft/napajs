//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This file contains application serving code for @napajs/vineyard.
//
// Each application sub-directory starts with an 'app.json', which is the root JSON file of this application. 
// The 'app.json' declares several aspects of an application, described by './schema/application-config.schema.json'. 
//
// Commonly defined aspects of an application are:
// 1) Object model - Facilitating object creation, provisioning, overriding and retrieval within application functions.
//    a. 'objectTypes': Define constructor of object types supported in current application.
//    b. 'objectProviders': Define URI based object providers in current applicaton.
//                          On provide() implementation, you can use two inputs, one is IObjectWithType and the other
//                          is IObjectWithType array. When you use an array as input, all items must be the same type.
//    c. 'namedObjects': Define objects that can be retrieved by global name in current application. 
//                       Like entry points of application, parameters etc.
//                       On construct() implementation, you have two input options, Uri or Uri array.
// 
//    In 'app.json' Each of these elements can include multiple separate JSON files, which enables sharing on 
//    common definitions across multiple applications. Overriding between multiple JSON files are also supported, 
//    an 'override' property needs to be set to true if we want to override an definition from entries 
//    in latter file against former ones. 
//
//    Common types and named objects are provided. Common types are "Function", "EntryPoint". "NamedObjectRef". 
//    And common named objects are shared commands such as "listAllEntryPoints", "listAllNamedObjects", etc.
//
//    All the concepts in object model could be overridden at request time, which enables us to alter system behavior
//    at request level. We can change system parameter by overriding a numeric named object, or experiment some ad-hoc
//    code by overriding a function named object in system. We can also override object constructor and providers to detour 
//    data creation from some hard dependencies to simple local implementations for testing purpose.
//
// 2) Application level resources,such as: 
//    a) Metrics
//    b) Logging
//    c) More.
//
// 3) Application policies, such as 
//    a) Throttle control policy.
//    b) More.
//
// Beyond the JSON definition that makes predefined application capability declarative, if there is any application specific
// properties for an application, application developers can always add properties to their applcation object in "<app-name>\app.ts".
//
// Application object is exposed as a global object, which resides in each V8 isolate and is alive since the application is initialized.
// Developers can retrieve a specific application by name via Application.getApplication(name) 
// or current application from RequestContext.getApplication().


// External dependencies.

// TODO: implement os.hostname(), it's used for debugging.
import * as os from 'os';
import * as path from 'path';
import * as assert from 'assert';

import * as napa from 'napajs';
import * as logger from '@napajs/logger';
import * as metrics from '@napajs/metrics';

// internal dependencies.
import * as objectContext from './object-context';
import * as wire from './wire';
import * as utils from './utils';
import * as config from './config'
import { NamedObject } from './named-object';

/////////////////////////////////////////////////////////////////////////////////////////
/// Application Engine for managing all applications.

/// <summary> Interface for global vinyard settings. </summary>
export interface Settings {
    /// <summary> Allow per-request override. Default is true. </summary>
    allowPerRequestOverride: boolean;

    /// <summary> Global scoped context definition. </summary>
    objectContext: objectContext.ScopedObjectContextDefinition;

    /// <summary> Default execution stack for all applications. Applications, entrypoints can override. </summary>
    defaultExecutionStack?: string[];

    /// <summary> Base directory to resolve relative paths. </summary>
    baseDir: string;
}

///////////////////////////////////////////////////////////////////////////////
/// Interfaces and classes for Application.

/// <summary> Interface for entrypoint function 
/// Entrypoint is a function that perform application logic.
/// Its return value will be property 'output' of Response. 
/// Entrypoint can be synchronous function or asynchronous functions. 
/// For asynchronous function, always return a Promise.
/// </summary>
export type EntryPoint = (requestContext?: RequestContext, input?: any) => any;


/// <summary> Interface for interceptor 
/// Interceptor is the execution unit of vinyard, multiple interceptors are stacked in execution,
/// That outer interceptors can short circuit the execution. This pattern is useful to add policy layers.
/// during execution, like throttle control, access control, instrumentation etc.
/// Following diagram depicts how multiple interceptors are organized together into an execution stack.
///
/// Execution stack:
///  +------------------------------------------------------------------------------+
///  | Interceptor 1          Do pre-request work 1                                 |
///  |               +-----------------------------------------------------------+  |
///  |               |  Interceptor 2    Do pre-request work 2                   |  |
///  |               |         +----------------------------------------------+  |  |
///  |               |         |   (More interceptors) ...                    |  |  |
///  |               |         |             +----------------------------+   |  |  |
///  |               |         |             |        Interceptor N       |   |  |  |
///  |               |         |             |     (Entrypoint execution) |   |  |  |
///  |               |         |             +----------------------------+   |  |  |
///  |               |         +----------------------------------------------+  |  |
///  |               |                   Do post-response work 2                 |  |
///  |               +-----------------------------------------------------------+  |
///  |                       Do post-response work 1                                |
///  +------------------------------------------------------------------------------+
///
/// Interceptor can be sync (always return a resolved promise) or async (with unresolved promise).
/// Always implement an interceptor in following patterns: 
/// 1) Do pre-request work (can be optional).
/// 2) call 'let response = await context.continueExecution()' or short circuit.
/// 3) Do post-request work (can be optional). 
/// </summary>
export type Interceptor = (context: RequestContext) => Promise<wire.Response>;

/// <summary> Interface for metric collection. </summary>
export type MetricCollection = { [name: string]: metrics.Metric };

/// <summary> Class for Metric definition. </summary>
export interface MetricDefinition {
    /// <summary> Name used to access this metric via context.metric['<name>'] </summary>
    name: string;

    /// <summary> Section e for the metric, which is passed to create the metric. </summary>
    sectionName: string;

    /// <summary> Display name for the metric, which is passed to create the metric. </summary>
    displayName: string;

    /// <summary> Description for this metric. For human consumption purpose. </summary>
    description?: string;

    /// <summary> Metric type. </summary>
    type: metrics.MetricType;

    /// <summary> Dimension definitions. </summary>
    dimensionNames?: string[];
}

/// <summary> Class for settings of an application </summary>
export interface ApplicationSettings extends Settings {
    /// <summary> ID of application. 
    /// To distinguish from name, which is associated with application instance at runtime by AppEngine.register(),
    /// ID is used for identifying the purpose of application, usually we can put module name as ID.
    /// </summary>
    id: string;

    /// <summary> Description of application. </summary>
    description?: string;

    /// <summary> Definition of metrics in this application. </summary>
    metrics: MetricDefinition[];
}

/// <summary> Class for Napa application. </summary>
export class Application {
    /// <summary> Application settings. </summary>
    private _settings: ApplicationSettings;

    /// <summary> Per-application object context. </summary>
    private _perAppObjectContext: objectContext.ScopedObjectContext;

    /// <summary> Default execution stack if not specified per-entrypoint. </summary>
    private _defaultExecutionStack: Interceptor[];

    /// <summary> Metric collection. </summary>
    private _metrics: MetricCollection;

    /// <summary> Per-entrypoint execution stack when there is no per-request override. </summary>
    private _perEntryPointExecutionStack: Map<string, Interceptor[]>;

    /// <summary> Construct application from application settings. </summary>
    /// <param name="engine"> Application engine that run current application. </summary>
    /// <param name="settings"> Application settings. </summary>
    public constructor(
        parentContext: objectContext.ScopedObjectContext,
        settings: ApplicationSettings) {
        
        this._settings = settings;
        this._perAppObjectContext = new objectContext.ScopedObjectContext(
            "application",
            settings.baseDir,
            parentContext,
            settings.objectContext);

        // Create default execution stack.
        this._defaultExecutionStack = [];

        // Prepare default execution stack.
        for (let interceptorName of this._settings.defaultExecutionStack) {
            let interceptor = this.getInterceptor(interceptorName);
            if (interceptor == null) {
                throw new Error("Interceptor does not exisit: '" + interceptorName + "'.");
            }
            this._defaultExecutionStack.push(interceptor);
        }

        // Prepare per-entrypoint execution stack.
        this._perEntryPointExecutionStack = new Map<string, Interceptor[]>();
        this._perAppObjectContext.forEach(object => {
            if (object.definition.value._type === 'EntryPoint') {
                let executionStack: Interceptor[] = this._defaultExecutionStack;
                let customStack = object.definition.value.executionStack;
                
                // Entrypoint has specified executionStack.
                if (customStack != null) {
                    executionStack = [];
                    for (let interceptorName of <string[]>(customStack)) {
                        let interceptor = this.getInterceptor(interceptorName);
                        if (interceptor == null) {
                            throw new Error("Interceptor does not exist: '" + interceptorName + "'");
                        }
                        executionStack.push(interceptor);
                    }
                }
                this._perEntryPointExecutionStack.set(
                    object.definition.name, 
                    executionStack);
            }
        });

        // Create metrics.
        this._metrics = {};
        if (settings.metrics != null) {
            for (let metric of settings.metrics) {
                this._metrics[metric.name] = metrics.get(
                    metric.sectionName,
                    metric.displayName,
                    metric.type,
                    metric.dimensionNames);
            }
        }
    }

    /// <summary> Get application ID. </summary>
    /// <returns> Application ID. </returns>
    public get id(): string {
        return this._settings.id;
    }

    /// <summary> Get application description. </summary>
    /// <returns> Application description. </returns>
    public get description(): string {
        return this._settings.description;
    }

    /// <summary> Get Application settings. </summary>
    /// <returns> Application settings. </returns>
    public get settings(): ApplicationSettings {
        return this._settings;
    }

    /// <summary> Get application level object context. </summary>
    /// <returns> Application level object context. </returns>
    public get objectContext(): objectContext.ScopedObjectContext {
        return this._perAppObjectContext;
    }

    /// <summary> Get default execution stack. </summary>
    /// <returns> Interceptor list configured as default stack. </returns>
    public get defaultExecutionStack(): Interceptor[] {
        return this._defaultExecutionStack;
    }

    /// <summary> Get execution stack for an entrypoint before any request override. </summary>
    /// <param name="entryPointName"> Entrypoint name. </param>
    /// <returns> Execution stack. </returns> 
    public getExecutionStack(entryPointName: string): Interceptor[] {
        if (this._perEntryPointExecutionStack.has(entryPointName)) {
            return this._perEntryPointExecutionStack.get(entryPointName);
        }
        return null;
    }
    /// <summary> Get metric collection of this application. </summary>
    /// <returns> Metric collection of current application. </summary>
    public get metrics(): MetricCollection {
        return this._metrics;
    }

    /// <summary> Create object from input. Throw exception if creation failed. </summary>
    /// <param name="input"> Any JS value </param>
    /// <returns> JS value created. </returns>
    public create(input: any): any {
        return this._perAppObjectContext.create(input);
    }

    /// <summary> Get the value of a named object. </summary>
    /// <param name='name'> Name of the object. Case sensitive. </param>
    /// <returns> Value of the named object or null if not found. </returns>
    public get(name: string): any {
        let namedObject = this.getNamedObject(name);
        if (namedObject != null) {
            return namedObject.value;
        }
        return null;
    }

    /// <summary> Get application level named object. </summary>
    /// <param name="name"> Name. Case-sensitive. </param>
    /// <returns> Named object if found. Otherwise undefined. </returns>    
    public getNamedObject(name: string): NamedObject {
        return this._perAppObjectContext.get(name);
    }

    /// <summary> Get entry point from current application. Throws exception if entry point is not found. </summary>
    /// <param name="entryPointName"> Entry point name, case sensitive. </param>
    /// <returns> Entrypoint (function) if found. Otherwise throws exception. </returns>
    public getEntryPoint(entryPointName: string): EntryPoint  {
        let object = this.getNamedObject(entryPointName);
        if (object != null && object.definition.value._type != 'EntryPoint') {
            throw new Error("Object '" + entryPointName + "' is not of EntryPoint type. Encountered: '" + object.definition.name + "'.");
        }
        return object != null? object.value : null;
    }

    /// <summary> Get interceptor by name. </summary>
    /// <param name="name"> Interceptor name. </param>
    /// <returns> Interceptor object if found, undefined otherwise. </returns>
    public getInterceptor(name: string): Interceptor {
        let object = this.getNamedObject(name);
        if (object != null && object.definition.value._type !== 'Interceptor') {
            throw new Error("Object '" + name + "' is not of Interceptor type. Encountered: '" + object.definition.name + "'.");
        }
        return object == null? null: object.value;
    }

    /// <summary> Get a function as a named object from current application. Throws exception if function is not found or not a function object. </summary>
    /// <param name="functionName"> Function name, case sensitive. </param>
    /// <returns> Function object if found. Otherwise throws exception. </returns>
    public getFunction(functionName: string): any {
        let object = this.getNamedObject(functionName);
        let fun = object == null ? null : object.value;
        if (fun != null && typeof fun !== 'function') {
            throw new Error("Object '" + functionName + "' is not a function.");
        }
        return fun;
    }
}

///////////////////////////////////////////////////////////////////////
/// Classes for request serving.

/// <summary> Class for request context.
/// Request context is the access point for all vineyard capabilities during serving a request.
/// These capabilities are:
/// 1) Object creation and accesses: via create() and get()/getNamedObject().
/// </summary>
export class RequestContext {
    /// <summary> Application</summary>
    private _application: Application = null;

    /// <summary> Request</summary>
    private _request: wire.Request = null;

    /// <summary> Entry point  </summary>
    private _entryPoint: EntryPoint = null;

    /// <summary> Request level object context. </summary>
    private _perRequestObjectContext: objectContext.ScopedObjectContext = null;

    /// <summary> Per request logger. </summary>
    private _logger: RequestLogger = null;

    /// <summary> Debugger. </summary>
    private _debugger: Debugger = null; 

    /// <summary> Execution state: current depth in execution stack. </summary>
    private _executionDepth: number = 0;

    /// <summary> Execution stack.
    private _executionStack: Interceptor[];
    
    /// <summary> Constructor </summary>
    public constructor(app: Application, request: wire.Request) {
        // Fill default values and do schema validation.
        request = wire.RequestHelper.fromJsValue(request);
        
        this._application = app;
        this._request = request;

        // We only pass overriden stuff when per-request override is allowed.
        let perRequestObjectContextDef: objectContext.ScopedObjectContextDefinition = 
            app.settings.allowPerRequestOverride ?
                new objectContext.ScopedObjectContextDefinition(
                    app.settings.objectContext,
                    request.overrideTypes,
                    request.overrideProviders,
                    request.overrideObjects,
                    false                           // Don't do dependency analysis at request level.
                )
                :
                new objectContext.ScopedObjectContextDefinition(
                    app.settings.objectContext,
                    [],
                    [], 
                    [], 
                    false);

        this._perRequestObjectContext = new objectContext.ScopedObjectContext(
            "request",
            app.settings.baseDir,
            app.objectContext,
            perRequestObjectContextDef);
        
        // Prepare execution stack and entry point.
        this._entryPoint = this.getEntryPoint(request.entryPoint);
        if (this._entryPoint == null) {
            throw new Error("Entrypoint does not exist: '" + request.entryPoint + "'");
        }

        this._executionStack = this.prepareExecutionStack(request.entryPoint);

        // Prepare logger and debuger.
        this._logger = new RequestLogger(
            request.application + "." + request.entryPoint,
            request.traceId);

        this._debugger = new Debugger();

        // Set execution depth to 0 to be at top of execution stack.
        this._executionDepth = 0;
    }

    /// <summary> prepare execution stack for an entrypoint name, assuming per-request object context is setup. </summary>
    private prepareExecutionStack(entryPointName: string): Interceptor[] {
        // If nothing has been overrided, use cached execution stack directly. 
        if (this._perRequestObjectContext.definition.namedObjects.length == 0) {
            if (this._entryPoint == null) {
                throw new Error("Entrypoint '" + entryPointName + "' does not exist.");
            }
            return this._application.getExecutionStack(entryPointName);
        } 

        // Per-request override happens, it could be entrypoint override or interceptor override.
        let entryPointObject = this.getNamedObject(entryPointName);
        let interceptorNames: string[] = entryPointObject.definition.value.executionStack;
        if (interceptorNames == null) {
            interceptorNames = this.application.settings.defaultExecutionStack;
        }

        // When entrypoint is not overriden, check if interceptor definition has be overriden.
        if (entryPointObject.scope !== 'request') {
            let oldStack = this.application.getExecutionStack(entryPointName);
            let newStack: Interceptor[] = [];
        
            // Definition and pre-cached execution stack should align.
            assert(oldStack.length == interceptorNames.length);

            for (let i = 0; i < oldStack.length; ++i) {
                let interceptorName = interceptorNames[i];
                let interceptorObject = this.getNamedObject(interceptorName);
                if (interceptorObject == null) {
                    throw("Interceptor '" + interceptorName + "' does not exist.");
                }
                
                if (interceptorObject.scope !== 'request') {
                    newStack.push(oldStack[i]);
                }
                else {
                    // Interceptor is overriden from request. Look up new.
                    if (interceptorObject.value == null
                        || interceptorObject.definition.value._type !== 'Interceptor') {
                        throw new Error("Bad override on interceptor '" 
                            + interceptorName 
                            + "', should be of Interceptor type and not null. ")
                    }
                    newStack.push(interceptorObject.value);
                }
            }
            return newStack;
        }

        // Per-request override happens on current entry point.
        let newStack: Interceptor[] = [];
        for (let interceptorName of interceptorNames) {
            let interceptor = this.getInterceptor(interceptorName);
            if (interceptor == null) {
                throw new Error("Interceptor '" + interceptorName + "' is not a valid interceptor.");
            }
            newStack.push(interceptor);
        }
        return newStack;
    }

    ///////////////////////////////////////////////////////////////////
    /// Operational interfaces

    /// <summary> Execute current request with a promise of response. </summary>
    public async execute(): Promise<wire.Response> {
        return this.continueExecution();
    }

    /// <summary> Continue execution from current interceptor. </summary>
    public async continueExecution(): Promise<wire.Response> {
        if (this._executionDepth < this._executionStack.length) {
            return this._executionStack[this._executionDepth++](this);
        }
        return Promise.resolve({ responseCode: wire.ResponseCode.Success });
    }

    ///////////////////////////////////////////////////////////////////
    /// Informational interfaces

    /// <summary> Get application of current request. </summary>
    public get application(): Application {
        return this._application;
    }

    /// <summary> Get the request used to create this context. </summary>
    public get request(): wire.Request {
        return this._request;
    }

    /// <summary> Entrypoint </summary>
    public get entryPoint(): EntryPoint {
        return this._entryPoint;
    }

    /// <summary> Get entry point name. </summary>
    public get entryPointName(): string {
        return this._request.entryPoint;
    }

    /// <summary> Get trace ID of current request. </summary>
    public get traceId(): string {
        return this._request.traceId;
    }

    /// <summary> Get control flags. </summary>
    public get controlFlags(): wire.ControlFlags {
        return this._request.controlFlags;
    }

    /// <summary> getter for metric collection. </summary>
    public get metric(): MetricCollection {
        return this._application.metrics;
    }

    /// <summary> Get input for entry point. </summary>
    public get input(): any {
        return this._request.input;
    }

    /// <summary> Get per request logger. </summary>
    public get logger(): RequestLogger {
        return this._logger;
    }

    /// <summary> Get debug info writter. </summary>
    public get debugger(): Debugger {
        return this._debugger;
    }

    ///////////////////////////////////////////////////////////////
    /// Behavioral interfaces
    
    /// <summary> Create object from input. </summary>
    /// <param name='input'> Input for creating object with type or URI </param>
    /// <returns> Created object. </returns>
    public create(input: any): any {
        return this._perRequestObjectContext.create(input);
    }

    /// <summary> Get the value of a named object. </summary>
    /// <param name='name'> Name of the object. Case sensitive. </param>
    /// <returns> Value of the named object or null if not found. </returns>
    public get(name: string): any {
        let namedObject = this.getNamedObject(name);
        if (namedObject != null) {
            return namedObject.value;
        }
        return null;
    }

    /// <summary> Get named object from input. </summary>
    /// <param name='name'> Name of the object. Case sensitive. </param>
    /// <returns> Named object or null if not found. </returns>
    public getNamedObject(name: string): NamedObject {
        return this._perRequestObjectContext.get(name);
    }

    /// <summary> Helper method to get entry point from application of request context. Throws exception if entry point is not found. </summary>
    /// <param name="entryPointName"> Entry point name, case sensitive. </param>
    /// <returns> Entrypoint (function) if found. Otherwise throws exception. </returns>
    public getEntryPoint(entryPointName: string): EntryPoint  {
        let object = this.getNamedObject(entryPointName);
        if (object != null && object.definition.value._type != 'EntryPoint') {
            throw new Error("Object '" + entryPointName + "' is not of EntryPoint type.");
        }
        return object != null? object.value : null;
    }

    /// <summary> Get interceptor by name. </summary>
    /// <param name="name"> Interceptor name. </param>
    /// <returns> Interceptor object if found, undefined otherwise. </returns>
    public getInterceptor(name: string): Interceptor {
        let object = this.getNamedObject(name);
        if (object != null && object.definition.name !== 'Interceptor') {
            throw new Error("Object '" + name + "' is not of Interceptor type.");
        }
        return object == null? null: object.value;
    }

    /// <summary> Helper method to get function from application of request context. Throws exception if function is not found or not a function object. </summary>
    /// <param name="functionName"> Function name, case sensitive. </param>
    /// <returns> Function object or null. If object associated with functionName is not a function, exception will be thrown. </returns>
    public getFunction(functionName: string): any {
        let func = this.get(functionName);
        if (func != null && typeof func !== 'function') {
            throw new Error("Object '" + functionName + "' is not a function.");
        }
        return func;
    }
}

/// <summary> Class for debugger, which writes debugInfo in response. </summary>
export class Debugger {
    /// <summary> Set last error that will be output in debug info. </summary>
    public setLastError(lastError: Error) {
        this._lastError = lastError;
    }

    /// <summary> Output an object with a key in debugInfo/details. </summary>
    public detail(key: string, value: any): void {
        this._details[key] = value;
    }

    /// <summary> Add a debug event with a log level and message. </summary>
    public event(logLevel: string, message: string): void {
        this._events.push({
            eventTime: new Date(),
            logLevel: logLevel,
            message: message
        });
    }

    /// <summary> Finalize debug info writer and return a debug info. </summary>
    public getOutput(): wire.DebugInfo {
        return  {
            exception: {
                message: this._lastError.message,
                stack: this._lastError.stack,
            },
            details: this._details,
            events: this._events,
            machineName: os.hostname(),
        };
    }

    private _lastError: Error = null;
    private _details: {[key: string]: any} = {};
    private _events: wire.DebugEvent[] = [];
}

/// <summary> Request logger that encapsulate section name and trace ID. </summary>
export class RequestLogger {
    public constructor(sectionName: string, traceId: string) {
        this._sectionName = sectionName;
        this._traceId = traceId;
    }

    /// <summary> Log message with Debug level. </summary>
    public debug(message: string) {
        logger.debug(this._sectionName, this._traceId, message);
    }

    /// <summary> Log message with Info level. </summary>
    public info(message: string) {
        logger.info(this._sectionName, this._traceId, message);
    }

    /// <summary> Log message with Warn level. </summary>
    public warn(message: string) {
        logger.warn(this._sectionName, this._traceId, message);
    }

    /// <summary> Log message with Error level. </summary>
    public err(message: string) {
        logger.err(this._sectionName, this._traceId, message);
    }

    private _traceId: string;
    private _sectionName: string;
}