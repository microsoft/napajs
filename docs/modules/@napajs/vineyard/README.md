# @napajs/vineyard: Application framework for building highly iterative applications

TODO: move this to modules/@napajs/vineyard/README.md later.

## Introduction
There are already many application frameworks under Node.JS, like express.js, etc. Why do we need another application framework? 

The short answer is: @napajs/vineyard is to solve different problems. 

As we introduced `napajs`, its major driving scenario was to support intelligent services, which use extensive rules, handcrafted features and machine learned models. This types of services share an experimental nature, which requires fast iteration on changing parameters, replacing algorithms, or even modifying code flows and getting feedback quickly. `@napajs/vineyard` is designed as a JSON server that enable applications with the ability of changing behaviors at runtime with maximized flexibility. It doesn't deal with presentation (like HTML, etc.), but solely focused on providing dynamic bindings on parameters, objects and functions during application execution.

Please note, @napajs/vineyard is a general framework that runs on both Napa containers and Node.JS. It also provides ability to serve applications across Napa containers and Node.JS in the same process.

## Quick Start

### **A simple use case**: Running an applcation under current running environment (Node.JS event loop or Napa container)

```typescript
import vy = require('@napajs/vineyard');

vy.register('@napajs/example-app', ['example']);

var request: vy.Request = {
    application: 'example',
    entrypoint: 'echo',
    input: 'hello, world'
};

var response: vy.Response = await vy.process(request);
console.log(JSON.stringify(response))

```

### **A more complex use case**: Running an application under Node.JS, dispatching to multiple Napa containers.

```typescript
import napa = require('napajs');
import vy = require('@napajs/vineyard');

napa.initialize();

// By using multiple container, we can define different runtime policies.
// Create container1 with default settings.
var container1 = napa.createContainer();

// Create container2 with customized settings.
var container2 = napa.createContainer({
        cores: 4,
        maxStackSize: 1024000,
        loggingProvider: '@napajs/ms-autopilot'
    });


// Serve an io-intensive-app in Node.JS eventloop.
vy.register('@napajs/io-intensive-app', ['example1'])

// Serve example-app2 using name 'example2' and example-app3 using name 'example3a' in container1. 
vy.register('@napajs/example-app2', ['example2'], container1);
vy.register('@napajs/example-app3', ['example3a'], container1);

// Serve example-app3 using name 'example3b' and example-app4 using name 'example4' in container2. 
vy.register('@napajs/example-app3', ['example3b'], container2);
vy.register('@napajs/example-app4', ['example4'], container2);

var request: vy.Request = {
    application: 'example1',
    entrypoint: 'echo',
    input: 'hello, world'
};

var response: vy.Response = await vy.process(request);
console.log(JSON.stringify(response));

```
-----
## Request and Response

`Request`
```json
{
    // Required. Application name, which is a value of the names passed to vy.register().
    "application": "<application-name>",

    // Required. A registered named object of 'Entrypoint' type. see [Entrypoint](#Entrypoint)
    "entryPoint": "<entrypoint-name>",

    // Optional. Input object for the entrypoint function, the value will be undefined if not specified.
    "input": {},

    // Optional. Control flags for request serving.
    "controlFlags": {
        // Optional. Output debugInfo in response. Default set to false.
        "debug": true,

        // Optional. Output perfInfo in response. Default set to false.
        "perf": true
    },

    // Optional. Override named object consumed by vy.RequestContext.get.
    // Through this mechanism, we can change parameters, objects and functions when entrypoint code against named objects.
    // Multiple named objects can be overriden in one request.
    "overrideObjects": [
        {
            // Required: name of the object. It should correspond to the name consumed in code.
            "name": "<object-name>",

            // Required: new value of the object. Can be primitive types, object or input to object creator and providers.
            "value": {
            }
        }
    ],

    // Optional. Override object retrieved by vy.ObjectContext.create.
    // see [Object Type](#objectype)
    "overrideTypes": [
        {
            // Required: type name.
            "type": "<type-name>",

            // Required: module name for the constructor function.
            "module": "<module-name>",

            // Required: name of the function as constructor.
            "constructor": "<function-name-as-constructor>"
        }
    ],

    // Optional. Override object (with URI) retrived by vy.ObjectContext.create.
    "overrideProviders": [
        {
            // Required: protocol name. such as 'doc' for URI "doc://<key>".
            "protocol": "<protocol-name>",

            // Required: module name that defines the function.
            "module": "<module-name>",

            // Required: function name.
            "function": "<function-name>"
        }
    ]
}
```
`Response`
```json
{
    // Status code of response.
    // Always present.
    "responseCode": 0,

    // Error message indicate why the request failed.
    // Present when responseCode is not 0.
    "errorMessage": "<error-message-if-any>",

    // Returned object (can be null, primitive type, or object type) from entrypoint.
    // Present when returned object is not 'undefined'.
    "output": {},

    // Present when request.controlFlags.debug is set to true.
    "debugInfo": {
        // Exception details when responseCode is not 0.
        "exception": {
            "stack": "<call-stack>",
            "message": "<exception-description>",
            "fileName": "<source-file-name>",
            "lineNumber": 0,
            "columnNumber": 0
        },
        // Logging events when called vy.RequestContext.logger.debug/info/warn/err.
        "events": [
            {
                "time": "<event-time>",
                "logLevel": "<log-level>",
                "message": "<message>"
            }
        ],
        // Probing values when called vy.RequestContext.logger.detail(<key>, <value>).
        "details": {
            "<debug-key>": "<debug-value>"
        }
    },

    // Present when request.controlFlags.perfInfo is set to true.
    "perfInfo": {
        "processingLatencyMS": 123
    }
}
```
-----
## Writing applications
TODO: use a realworld case as example.
### Step 1: Coding the logics
`example-app/example.ts`
```typescript
import app = require('@napajs/vineyard');

////////////////////////////////////////////////////////////////////////////
/// Functions for entrypoints.

/// Function for entrypoint 'echo'. 
/// See 'named-objects.json' below on how we register this entrypoint.
/// The 1st parameter of an entrypoint is the input from request.
/// The 2nd parameter is an vy.RequestContext object.
/// In 'echo', we don't need to access request context, so simply not to declare it.
export function echo(text: string) {
    return text;
}

/// Function for entrypoint 'compute', which is to compute sum on an array of numbers. 
/// Since we support computation function override, which needs to access the vy.RequestContext object,
/// we declare vy.RequestContext as the 2nd parameter of 'compute'.
export function compute(numberArray: number[], context: vy.RequestContext) {
    var func = (list: number[]) => {
        return list.reduce((sum: number, value: number) => {
                return sum + value;
            }, 0);
    }
    // Note: context.get will returned named object giving a name.
    var functionObject = context.get('customFunction');
    if (functionObject != null) {
        func = functionObject.value;
    }
    return func(numberArray);
}

/// Function for entrypoint 'loadObject', which return an object for the uri.
/// NOTE: We use URI to represent object that is able to reference and share more conveniently.
export function loadObject(uri: string, context: vy.RequestContext) {
    // Note: context.create will detect uri string and use registered object provider to create the object.
    return context.create(uri);
}

/// Function that will be used to provide objects for protocol 'text'.
export function createObject(input: any, context: vy.RequestContext) {
    // Note: for non-uri input, context.create will use constructor of registered object types to create it.
    return context.create(input);
}

////////////////////////////////////////////////////////////////////////////
/// Functions for object types.
TODO:

////////////////////////////////////////////////////////////////////////////
/// Functions for object providers.
TODO:

```
### Step 2: Configuring Things Together
`example-app/vy.json` (root configuration)
```json
{
    "id": "example-app",
    "description": "Example application for @napajs/vineyard",
    "objectTypes": ["./object-types.json"],
    "objectProviders": ["./object-providers.json"],
    "namedObjects": ["./named-objects.json"],
    "interceptors": ["./interceptors.json"]
}
```
`example-app/object-types.json` (a configuration file for objectTypes)

See [[Object Type]](#object-type).

```json
[
    {
        "type": "<type-name>",
        "description": "<type-description>",
        "module": "<module-name>",
        "constructor": "<function-name-as-constructor>"
    }
]

```

`example-app/object-providers.json` (a configuration file for objectProviders)

See [[Object Provider]](#object-provider)
```json
[
    {
        "protocol": "<protocol-name>",
        "description": "<protocol-description>",
        "module": "<module-name>",
        "function": "<function-name-as-loader>"
    }
]

```
`example-app/named-objects.json` (a configuration file for namedObjects)

See [[Named Object]](#named-object)
```json
[
    {
        "name": "<object-name>",
        // Object value can be created by object factory or providers.
        "value": {}
    }
]

```
### Step 3 - Trying requests
```json
TODO
```
-----
## Concepts Explained
The core of @napajs/vineyard is to provide mechanisms for overriding behaviors. As behaviors are encapsulated into objects (primitive types, complex types, functions, classes, etc.), overriding behavior would simply be overriding objects. 

There are two requirements on overriding objects.
- Override an specific object: We introduced [Named Object](#named-object) to access object by a string key, we can also use the key to override the object.
- Override objects of the same type or provisioned by the same protocol: [Object Type](#object-type) and [Object provider](#object-provider) are designed to satisfy this need.

[Object Context](#object-context) is an interface to expose object creation, provisioning and access-by-name behaviors. @napajs/vineyard implemented a chain of Object Context at different scope (application, request time, etc.) to fulfill an efficient request time overriding.

### Object Context
```typescript
export interface ObjectContext {
    /// Create an object from any input.
    /// If input is an URI string, it invokes the provider to create the object.
    /// If input is an typed object, it invokes the constructor assigned to the type
    create(input: any): any;

    /// Get an object by name.
    get(name: string): any;
}
```
### Object Type
**Interfaces**: ObjectWithType, ObjectConstructor and ObjectFactory
```typescript
/// An object with type has a string property '_type'. 
export interface ObjectWithType {
    _type: string;
}

/// Object constructor is registered to create object for a specific type.
/// It can access the object context for creating nested objects.
export interface ObjectConstructor {
    (input: ObjectWithType | ObjectWithType[], context?: ObjectContext): any;
}

/// Object factory interface, which register many ObjectConstructor with many types.
export interface IObjectFactory {

    /// <summary> Construct an output JS value from input object. </summary>
    /// <param name="input"> Object with '_type' property or object array. </param>
    /// <param name="context"> Context if needed to construct sub-objects. </param>
    /// <returns> Any JS value type. </returns>
    /// <remarks>
    /// When input is array, all items in array must be the same type.
    /// On implementation, you can check whether input is array or not as Array.isArray(input).
    /// Please refer to example\example_types.ts.
    /// </remarks>
    construct(input: IObjectWithType | IObjectWithType[], context?: IObjectContext): any;

    /// <summary> Check whether current object factory support given type. </summary>
    /// <param name="typeName"> value of '_type' property. </param>
    /// <returns> True if supported, else false. </returns>
    supports(typeName: string): boolean;
}

```
**Registration**: How object type is associated with a object constructor in `object-types.json`
```json
[
    {
        "type": "<type-name>",
        "description": "<description-of-type>",
        "module": "<module-name>",
        "constructor": "<constructor-function-name>"
    }
]
```
**Usage**: How object type is invoved in object creation.
```typescript
    var func = context.create({
            "_type": "Function",
            "function": "function(a, b){ return a + b; }"
        });

    // Will print '3'.
    console.log(func(1, 2))
    
```

#### Built-in types.
##### Function
A predefined function object.
```json
{
    "_type": "Function",
    "module": "a-module",
    "function": "aFunction"
}
```
or an embeded JavaScript function definition:

```json
{
    "_type": "Function",
    "function": "function (a, b) { return a + b; }"
}

```

##### Entrypoint
Entrypoint type is a specific function that can be used as entrypoint.
```typescript
export interface EntryPoint {
    (input?: any, requestContext?: RequestContext): any
}
```
JSON input to construct an entrypoint.
```json
{
    "_type": "Entrypoint",
    "module": "a-module",
    "function": "aEntrypoint"
}
```
### Object Provider
Object provider is similar to object constructor, instead of working on object of a type, it creates objects based on protocol from a URI.
For example, in URI "doc://abcde", "doc" is the protocol, "abcde" is path that carry information on what/how the object can be created.

The reason we introduce URI based objets is to advocate a human-readable way to identify and share objects.

**Interfaces**: ObjectLoader and ObjectProvider
```typescript

/// Function to load (create) an object from Uri.
export interface ObjectLoader {
    (uri: Uri | Uri[], context?: ObjectContext): any;
}

/// Interface for ObjectProvider.
export interface ObjectProvider {
    /// <summary> Provide any JS value from a URI. </summary>
    /// <param name="uri"> a URI object or array of URIs. </param>
    /// <param name="context"> Object context if needed to create sub-objects. </param>
    /// <returns> Any JS value. </returns>
    /// <remarks>
    /// On implementation, you can check whether input is array or not as Array.isArray(input).
    /// Please refer to example\example_providers.ts.
    /// </remarks>
    provide(uri: Uri | Uri[], context?: ObjectContext): any;

    /// <summary> Check if current provider support a protocol name.</summary>
    /// <param name="protocol"> Case insensitive protocol name. </param>
    /// <returns> True if protocol is supported, otherwise false. </param>
    supports(protocol: string): boolean;
}

```
**Registration**: protocol and object provider are associated together via `object-providers.json` from 'objectProviders' element of root configuration.
```json
[
    {
        "protocol": "<protocol-name>",
        "description": "<protocol-description>",
        "module": "<module-name>",
        "function": "<function-name-as-loader>"
    }
]

```
**Usage**: How object provider is invovled in object creation.
```typescript
    var doc = context.create('doc://example-doc');
    
    // Will print { "id": "example-doc-01", title: "xxx", ...}
    console.log(JSON.stringify(doc));
```
### Named Object
Named object is introduced to access well-known objects in system by a string name. 

**Interfaces**: NamedObjectDefinition, NamedObject and NamedObjectCollection
```typescript

export class NamedObjectDefinition {
    public name: string = null;
    public description: string = null;
    public isPrivate: boolean = false;
    public override: boolean = false;
    public value: any = null;
}

export class NamedObject {
    
    /// <summary> Constructor </summary>
    /// <param name="definition"> Definition of current named object. </param>
    /// <param name="value"> Value of current named object </param>
    public constructor(public definition: NamedObjectDefinition, public value: any) {
        this.definition = definition;
        this.value = value;
    }
}

export interface INamedObjectCollection {
    /// <summary> Get named object by name. </summary>
    /// <param name="name"> Name. Case-sensitive. </summary>
    /// <returns> Named object if found. Otherwise undefined. </returns>
    get(name: string): NamedObject;
}

```
**Registration**: Registering named object in `named-objects.json`.

Named object can be created automatically from JSON (with the support of ObjectType and ObjectProvider). In configuration, you can register named objects in `named-objects.json`, which is included from 'namedObjects' element of root configuration.

Please note: Entrypoint registration is simply a named object registration.

```json
[
    {
        "name": "echo",
        // Entrypoint, created as an object with type (Entrypoint).
        "value": {
            "_type": "EntryPoint",
            "module": "./example",
            "function": "echo"
        }
    },
    {
        "name": "example-doc",
        // Object created by object provider for protocol "doc".
        "value": "doc://example-document"
    }
]
```
**Usage**: How named object is involved in object access.
```typescript
var object = context.get('example-doc');

console.log(JSON.)
```
----
*End of document*