# namespace `zone`

## Table of Contents
- [`create(id: string, settings: ZoneSettings = DEFAULT_SETTINGS): Zone`](#create-id-string-settings-zonesettings-default_settings-zone)
- [`getOrCreate(id: string, settings: ZoneSettings = DEFAULT_SETTINGS): Zone`](#getOrCreate-id-string-settings-zonesettings-default_settings-zone)
- [`get(id: string): Zone`](#get-id-string-zone)
- [`current: Zone`](#current-zone)
- [`node: Zone`](#node-zone)
- interface [`ZoneSettings`](#interface-zonesettings)
    - [`settings.workers: number`](#settings-workers-number)
- object [`DEFAULT_SETTINGS: ZoneSettings`](#object-default_settings-zonesettings)
- interface [`Zone`](#interface-zone)
    - [`zone.id: string`](#zone-id-string)
    - [`zone.broadcast(code: string): Promise<void>`](#zone-broadcast-code-string-promise-void)
    - [`zone.broadcast(function: (...args: any[]) => void, args: any[]): Promise<void>`](#zone-broadcast-function-args-any-void-args-any-promise-void)
    - [`zone.broadcastSync(code: string): void`](#zone-broadcastsync-code-string-void)
    - [`zone.broadcastSync(function: (...args: any[]) => void, args: any[]): void`](#zone-broadcastsync-function-args-any-void-args-any-void)
    - [`zone.execute(moduleName: string, functionName: string, args: any[], timeout: number): Promise<ExecuteResult>`](#zone-execute-modulename-string-functionname-string-args-any-timeout-number-promise-executeresult)
    - [`zone.execute(function: (...args[]) => any, args: any[], timeout: number): Promise<ExecuteResult>`](#zone-execute-function-args-any-args-any-timeout-number-promise-executeresult)
    - [`zone.executeSync(moduleName: string, functionName: string, args: any[], timeout: number): ExecuteResult`](#zone-executesync-modulename-string-functionname-string-args-any-timeout-number-executeresult)
    - [`zone.executeSync(function: (...args: any[]) => any, args: any[], timeout: number): ExecuteResult`](#zone-executesync-function-args-any-any-args-any-timeout-number-executeresult)
- interface [`ExecuteResult`](#interface-executeresult)
    - [`result.value: any`](#result-value-any)
    - [`result.payload: string`](#result-payload-string)
    - [`result.transportContext: transport.TransportContext`](#result-transportcontext-transport-transportcontext)

## API
### create(id: string, settings: ZoneSettings): Zone

It creates a Napa zone with a string id. If zone with the id is already created, error will be thrown. [`ZoneSettings`](#interface-zonesettings) can be specified for creating zones.

Example 1: Create a zone with id 'zone1', using default ZoneSettings. 
```ts
import * as napa from 'napajs'
let zone = napa.zone.create('zone1');
```
Example 2: Create a zone with id 'zone1', with 1 worker.
```ts
let zone = napa.zone.create('zone1', {
    workers: 1
});
```
### getOrCreate(id: string, settings: ZoneSettings): Zone
It gets a reference of zone by an id if a zone with the id already exists, otherwise create a new one and return its reference.

Example:
```ts
let zone = napa.zone.getOrCreate('zone1', {
    workers: 4
});
```
### get(id: string): Zone
It gets a reference of zone by an id. Error will be thrown if the zone doesn't exist.

Example:
```ts
let zone = napa.zone.get('zone1');
```
### current: Zone
It returns a reference of the zone of current running isolate. If it's under node, it returns the [node zone](#node).

Example: Get current zone.
```ts
let zone = napa.zone.current;
```
### node: Zone
It returns a reference to the node zone. It is equivalent to `napa.zone.get('node')`;

Example:
```ts
let zone = napa.zone.node;
```
## Interface `ZoneSettings`
Settings for zones, which will  be specified during creation of zones. If not specified, [DEFAULT_SETTINGS](#object-defaultSettings) will be used.

### settings.workers: number
Number of workers in the zone.

## Object `DEFAULT_SETTINGS`
Default settings for creating zones.
```ts
{
    workers: 2
}
```
## Interface `Zone`
Zone is the basic concept to execute JavaScript and apply policies in Napa. You can find its definition in [Architecture](../../README.md#architecture). Through Zone API, developers can broadcast JavaScript code on all workers, or execute a function on one of them. When you program against a zone, it is the best practice to ensure all workers within a zone are symmetrical to each other, that you should not assume a worker may maintain its own states.

The two major set of APIs are `broadcast` and `execute`, with both synchronous and asynchronous version plus a few variations on their inputs.
### zone.id: string
It gets the id of the zone.

### zone.broadcast(code: string): Promise\<void\>
It asynchronously broadcasts a snippet of JavaScript code in a string to all workers, which returns a Promise of void. If any of the workers failed to execute the code, promise will be rejected with an error message.

Example:

```ts
let zone = napa.zone.get('zone1');
zone.broadcast('var state = 0;')
    .then(() => {
        console.log('broadcast succeeded.');
    })
    .catch((error) => {
        console.log('broadcast failed.')
    });
```
### zone.broadcast(function: (...args: any[]) => void, args: any[]): Promise\<void\>
It asynchronously broadcasts an annoymous function with its arguments to all workers, which returns a Promise of void. If any of the workers failed to execute the code, promise will be rejected with an error message.

*Please note that Napa doesn't support closure in 'function' during broadcast.

Example:

```ts
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
### zone.broadcastSync(code: string): void
It synchronously broadcasts a snippet of JavaScript code in a string to all workers. If any of the workers failed to execute the code, an error will be thrown with message.

Example:

```ts
try {
    zone.broadcastSync("var state = 0;");
}
catch (error) {
    console.log('broadcast failed:', error);
}
```
### zone.broadcastSync(function: (...args: any[]) => void, args: any[]): void

It synchronously broadcasts an annoymous function with its arguments to all workers. If any of the workers failed to execute the code, an error will be thrown with message.

*Please note that Napa doesn't support closure in 'function' during broadcastSync.

Example:
```ts
try {
    zone.broadcastSync(() => {
            require('some-module').setModuleState(state)
        }, [{field1: 1}]);
}
catch (error) {
    console.log('broadcast failed:', error);
}
```
### zone.execute(moduleName: string, functionName: string, args: any[], timeout: number = 0): Promise\<any\>
Execute a function asynchronously on arbitrary worker via module name and function name. Arguments can be of any JavaScript type that is [transportable](transport.md#transportable-types). It returns a Promise of [`ExecuteResult`](#interface-executeresult). If error happens, either bad code, user exception, or timeout is reached, promise will be rejected.

Example: Execute function 'bar' in module 'foo', with arguments [1, 'hello', { field1: 1 }]. 300ms timeout is applied.
```ts
zone.execute('foo', 'bar', [1, "hello", {field1: 1}], 300)
    .then((result: ExecuteResult) => {
        console.log('execute succeeded:', result.value);
    })
    .catch((error) => {
        console.log('execute failed:', error);
    });

```

### zone.execute(function: (...args: any[]) => any, args: any[], timeout: number = 0): Promise\<any\>

Execute an anonymous function asynchronously on arbitrary worker. Arguments can be of any JavaScript type that is [transportable](transport.md#transportable-types). It returns a Promise of [`ExecuteResult`](#interface-executeresult). If error happens, either bad code, user exception, or timeout is reached, promise will be rejected.

Example:
```ts
zone.execute((a: number, b: string, c: object) => {
        return a + b + JSON.stringify(c);
    }, [1, "hello", {field1: 1}])
    .then((result: ExecuteResult) => {
        console.log('execute succeeded:', result.value);
    })
    .catch((error) => {
        console.log('execute failed:', error);
    });

```

### zone.executeSync(moduleName: string, functionName: string, args: any[], timeout: number = 0): any

Execute a function synchronously on arbitrary worker via module name and function name. Arguments can be of any JavaScript type that is [transportable](transport.md#transportable-types). It returns an [`ExecuteResult`](#interface-executeresult). If error happens, either bad code, user exception, or timeout is reached, error will be thrown.

Example: Execute function 'bar' in module 'foo', with arguments [1, 'hello', { field1: 1 }]. 300ms timeout is applied.
```ts
try {
    let result = zone.executeSync('foo', 'bar', [1, "hello", {field1: 1}], 300);
    console.log("execute succeeded:", result.value);
}
catch (error) {
    console.log("execute failed:", error);
}

```

### zone.executeSync(function: (...args: any[]) => any, args: any[], timeout: number = 0): any
Execute an annoymouse function synchronously on arbitrary worker. Arguments can be of any JavaScript type that is [transportable](transport.md#transportable-types). It returns an [`ExecuteResult`](#interface-executeresult). If error happens, either bad code, user exception, or timeout is reached, error will be thrown.

Example: Execute annoymouse function sychronously, with arguments [1, 'hello', { field1: 1 }]. No timeout is applied.
```ts
try {
    let result = zone.executeSync((a: number, b: string, c: object) => {
            return a + b + JSON.stringify(c);
        }, [1, "hello", {field1: 1}]);
    console.log("execute succeeded:", result.value);
}
catch (error) {
    console.log("execute failed:", error);
}

```

## Interface `ExecuteResult`
Interface to access return value of `zone.execute` or `zone.executeSync`.

### result.value: any
JavaScript value returned from function which is invoked from zone.execute/executeSync. Napa marshall/unmarshall [transportable values](transport.md#transportable-types) between different workers (V8 isolates). Unmarshalling will happen when the first `result.value` is queried.

Example:
```ts
let value = result.value;
```

### result.payload: string
Marshalled payload (in JSON) from returned value. This field is for users that want to pass result through to its caller, where unmarshalled value is not required.  

Example:
```ts
let payload = result.payload;
```

### result.transportContext: transport.TransportContext
[TransportContext](transport.md#transport-context) that is required to unmarshall [`result.payload`](#result-payload-string) into [`result.value`](#result-value-any).

Example:
```ts
import * as napa from 'napajs';

// Get result from zone.execute...

let transportContext = result.transportContext;
let value = napa.transport.unmarshall(result.payload, result.transportContext);
assert.equal(value, result.value);
```