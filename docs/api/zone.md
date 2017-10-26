# Namespace `zone`

## Table of Contents
- [Introduction](#intro)
    - [Multiple workers vs Multiple zones](#worker-vs-zone)
    - [Zone types](#zone-types)
    - [Zone operations](#zone-operations)
- [API](#api)
    - [`create(id: string, settings: ZoneSettings = DEFAULT_SETTINGS): Zone`](#create)
    - [`get(id: string): Zone`](#get)
    - [`current: Zone`](#current)
    - [`node: Zone`](#node-zone)
    - Interface [`ZoneSettings`](#zone-settings)
        - [`settings.workers: number`](#zone-settings-workers)
    - Object [`DEFAULT_SETTINGS: ZoneSettings`](#default-settings)
    - Interface [`Zone`](#zone)
        - [`zone.id: string`](#zone-id)
        - [`zone.broadcast(code: string): Promise<void>`](#broadcast-code)
        - [`zone.broadcast(function: (...args: any[]) => void, args?: any[]): Promise<void>`](#broadcast-function)
        - [`zone.execute(moduleName: string, functionName: string, args?: any[], options?: CallOptions): Promise<Result>`](#execute-by-name)
        - [`zone.execute(function: (...args[]) => any, args?: any[], options?: CallOptions): Promise<Result>`](#execute-anonymous-function)
    - Interface [`CallOptions`](#call-options)
        - [`options.timeout: number`](#call-options-timeout)
    - Interface [`Result`](#result)
        - [`result.value: any`](#result-value)
        - [`result.payload: string`](#result-payload)
        - [`result.transportContext: transport.TransportContext`](#result-transportcontext)

## <a name="intro"></a> Introduction
Zone is a key concept of napajs that exposes multi-thread capabilities in JavaScript world, which is a logical group of symmetric workers for specific tasks. 

Please note that it's not the same `zone` concept of a context object for async calls in [Dart](https://www.dartlang.org/articles/libraries/zones), or [Augular](https://github.com/angular/zone.js), or a proposal in [TC39](https://github.com/domenic/zones).

### <a name="worker-vs-zone"></a> Multiple workers vs. Multiple zones
Zone consists of one or multiple JavaScript threads, we name each thread `worker`. Workers within a zone are symmetric, which means code executed on any worker from the zone should return the same result, and the internal state of every worker should be the same from a long-running point of view. 
 
Multiple zones can co-exist in the same process, with each loading different code, bearing different states or applying different policies, like heap size, etc. The purpose of having multiple zone is to allow multiple roles for complex work, each role loads the minimum resources for its own usage.
 
### <a name="zone-types"></a> Zone types
There are two types of zone:
- **Napa zone** - zone consists of Napa.js managed JavaScript workers (V8 isolates). Can be multiple, each may contain multiple workers. Workers in Napa zone support partial Node.JS APIs.
- **Node zone** - a 'virtual' zone which exposes Node.js eventloop, has access to full Node.js capabilities.

### <a name="zone-operations"><a> Zone operations 
There are two operations, designed to reinforce the symmetry of workers within a zone:
 1) **Broadcast** - run code that changes worker state on all workers, returning a promise for the pending operation. Through the promise, we can only know if the operation succeed or failed. Usually we use `broadcast` to bootstrap the application, pre-cache objects, or change application settings.
 2) **Execute** - run code that doesn't change worker state on an arbitrary worker, returning a promise of getting the result. Execute is designed for doing the real work.

 Zone operations are on a basis of first-come-first-serve, while `broadcast` takes higher priority over `execute`.

## <a name="api"></a>API
### <a name="create"></a> create(id: string, settings: ZoneSettings): Zone

It creates a Napa zone with a string id. If zone with the id is already created, error will be thrown. [`ZoneSettings`](#zone-settings) can be specified for creating zones.

Example 1: Create a zone with id 'zone1', using default ZoneSettings. 
```js
var napa = require('napajs');
var zone1 = napa.zone.create('zone1');
```
Example 2: Create a zone with id 'zone2', with 1 worker.
```js
var zone2 = napa.zone.create('zone2', {
    workers: 1
});
```
### <a name="get"></a> get(id: string): Zone
It gets a reference of zone by an id. Error will be thrown if the zone doesn't exist.

Example:
```js
var zone = napa.zone.get('zone1');
```
### <a name="current"></a>current: Zone
It returns a reference of the zone of the currently running isolate. If it's under node, it returns the [node zone](#node-zone).

Example: Get current zone.
```js
var zone = napa.zone.current;
```
### <a name="node-zone"></a>node: Zone
It returns a reference to the node zone. It is equivalent to `napa.zone.get('node')`;

Example:
```js
var zone = napa.zone.node;
```
## <a name="zone-settings"></a> Interface `ZoneSettings`
Settings for zones, which will be specified during the creation of zones. If not specified, [DEFAULT_SETTINGS](#default-settings) will be used.

### <a name="zone-settings-workers"></a>settings.workers: number
Number of workers in the zone.

## <a name="default-settings"></a> Object `DEFAULT_SETTINGS`
Default settings for creating zones.
```js
{
    workers: 2
}
```
## <a name="zone"></a> Interface `Zone`
Zone is the basic concept to execute JavaScript and apply policies in Napa. You can find its definition in [Introduction](#intro). Through the Zone API, developers can broadcast JavaScript code on all workers, or execute a function on one of them. When you program against a zone, it is the best practice to ensure all workers within a zone are symmetrical to each other, that is, you should not assume a worker may maintain its own states.

The two major sets of APIs are [`broadcast`](#broadcast-code) and [`execute`](#execute-by-name), which are asynchronous operations with a few variations on their inputs.
### <a name="zone-id"></a> zone.id: string
It gets the id of the zone.

### <a name="broadcast-code"></a> zone.broadcast(code: string): Promise\<void\>
It asynchronously broadcasts a snippet of JavaScript code in a string to all workers, which returns a Promise of void. If any of the workers failed to execute the code, the promise will be rejected with an error message.

Example:

```js
var napa = require('napajs');
var zone = napa.zone.get('zone1');
zone.broadcast('var state = 0;')
    .then(() => {
        console.log('broadcast succeeded.');
    })
    .catch((error) => {
        console.log('broadcast failed.')
    });
```
### <a name="broadcast-function"></a> zone.broadcast(function: (...args: any[]) => void, args?: any[]): Promise\<void\>
It asynchronously broadcasts an anonymous function with its arguments to all workers, which returns a Promise of void. If any of the workers failed to execute the code, the promise will be rejected with an error message.

*Please note that Napa doesn't support closure in 'function' during broadcast.

Example:

```js
zone.broadcast((state) => {
        require('some-module').setModuleState(state)
    }, [{field1: 1}])
    .then(() => {
        console.log('broadcast succeeded.');
    })
    .catch((error) => {
        console.log('broadcast failed:', error)
    });
```
### <a name="execute-by-name"></a> zone.execute(moduleName: string, functionName: string, args?: any[], options?: CallOptions): Promise\<any\>
Execute a function asynchronously on an arbitrary worker via module name and function name. Arguments can be of any JavaScript type that is [transportable](transport.md#transportable-types). It returns a Promise of [`Result`](#result). If an error happens, either bad code, user exception, or timeout is reached, the promise will be rejected.

Example: Execute function 'bar' in module 'foo', with arguments [1, 'hello', { field1: 1 }]. 300ms timeout is applied.
```js
zone.execute(
    'foo', 
    'bar', 
    [1, "hello", {field1: 1}], 
    { timeout: 300 })
    .then((result) => {
        console.log('execute succeeded:', result.value);
    })
    .catch((error) => {
        console.log('execute failed:', error);
    });

```

### <a name="execute-anonymous-function"></a> zone.execute(function: (...args: any[]) => any, args?: any[], options?: CallOptions): Promise\<any\>

Execute a function object asynchronously on an arbitrary worker. Arguments can be of any JavaScript type that is [transportable](transport.md#transportable-types). It returns a Promise of [`Result`](#result). If an error happens, either bad code, user exception, or timeout is reached, promise will be rejected.

Here are a few restricitions on executing a function object:

- The function object cannot access variables from closure
- Unless the function object has an `origin` property, it will use the current file as `origin`, which will be used to set `__filename` and `__dirname`. (See [transporting functions](./transport.md#transporting-functions))

Example:
```js
zone.execute((a: number, b: string, c: object) => {
        return a + b + JSON.stringify(c);
    }, [1, "hello", {field1: 1}])
    .then((result) => {
        console.log('execute succeeded:', result.value);
    })
    .catch((error) => {
        console.log('execute failed:', error);
    });

```
Output:
```
execute succeeded: 1hello{"field1":1}

```
Another example demonstrates accessing `__filename` when executing an anonymous function:
```js
// File: /usr/file1.js
zone.execute(() => { console.log(__filename);});
```
Output:
```
/usr/file1.js
```
## <a name="call-options"></a> Interface `CallOptions`
Interface for options to call functions in `zone.execute`.

### <a name="call-options-timeout"></a> options.timeout: number
Timeout in milliseconds. Default value 0 indicates no timeout.

## <a name="result"></a> Interface `Result`
Interface to access the return value of [`execute`](#execute-by-name).

### <a name="result-value"></a>result.value: any
JavaScript value returned from the function which is invoked from zone.execute/executeSync. Napa marshalls/unmarshalls [transportable values](transport.md#transportable-types) between different workers (V8 isolates). Unmarshalling will happen when the first `result.value` is queried.

Example:
```js
var value = result.value;
```

### <a name="result-payload"></a> result.payload: string
Marshalled payload (in JSON) from the returned value. This field is for users that want to pass results through to its caller, where the unmarshalled value is not required.  

Example:
```js
var payload = result.payload;
```

### <a name="result-transportcontext"></a> result.transportContext: transport.TransportContext
[TransportContext](transport.md#transport-context) that is required to unmarshall [`result.payload`](#result-payload) into [`result.value`](#result-value).

Example:
```js
var napa = require('napajs');
var zone = napa.zone.create('zone1');
zone.execute(() => { return 0; }, [])
    .then((result) => {
        // Manually marshall.
        var transportContext = result.transportContext;
        var value = napa.transport.unmarshall(result.payload, result.transportContext);

        // result.value and manual unmarshall from payload are the same.
        assert.equal(value, result.value);
    });
```
