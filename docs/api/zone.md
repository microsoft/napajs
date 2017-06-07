# namespace `zone`

## Table of Contents
- [`create(id: string, settings: ZoneSettings = DEFAULT_SETTINGS): Zone`](#create)
- [`getOrCreate(id: string, settings: ZoneSettings = DEFAULT_SETTINGS): Zone`](#getOrCreate)
- [`get(id: string): Zone`](#get)
- [`current: Zone`](#current)
- [`node: Zone`](#node)
- interface [`ZoneSettings`](#zoneSettings)
    - [`settings.workers: number`](#settings-workers)
- [`DEFAULT_SETTINGS: ZoneSettings`](#defaultSettings)
- interface [`Zone`](#zone)
    - [`zone.id: string`](#zone-id)
    - [`zone.broadcast(code: string): Promise<void>`](#broadcast-code)
    - [`zone.broadcast(function: (...args: any[]) => void, args: any[]): Promise<void>`](#broadcast-function)
    - [`zone.broadcastSync(code: string): void`](#broadcastSync-code)
    - [`zone.broadcastSync(function: (...args: any[]) => void, args: any[]): void`](#broadcastSnc-function)
    - [`zone.execute(moduleName: string, functionName: string, args: any[], timeout: number): Promise<any>`](#execute-name)
    - [`zone.execute(function: (...args[]) => any, args: any[], timeout: number): Promise<any>`](#execute-function)
    - [`zone.executeSync(moduleName: string, functionName: string, args: any[], timeout: number): any`](#executeSync-name)
    - [`zone.executeSync(function: (...args: any[]) => any, args: any[], timeout: number): any`](#executeSync-function)
- interface [`ExecuteResult`](#executeResult)
    - [`result.value: any`](#result-value)
    - [`result.payload: string`](#result-payload)
    - [`result.transportContext: transport.TransportContext`](#result-transportContext)

## API
### <a name="create"></a>create(id: string, settings: ZoneSettings): Zone

It creates a Napa zone with a string id. If zone with the id is already created, error will be thrown. [`ZoneSettings`](#zoneSettings) can be specified for creating zones.

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
### <a name="getOrCreate"></a>getOrCreate(id: string, settings: ZoneSettings): Zone
It gets a reference of zone by an id if a zone with the id already exists, otherwise create a new one and return its reference.

Example:
```ts
let zone = napa.zone.getOrCreate('zone1', {
    workers: 4
});
```
### <a name="get"></a>get(id: string): Zone
It gets a reference of zone by an id. Error will be thrown if the zone doesn't exist.

Example:
```ts
let zone = napa.zone.get('zone1');
```
### <a name="current"></a>current: Zone
It returns a reference of the zone of current running isolate. If it's under node, it returns the [node zone](#node).

Example: Get current zone.
```ts
let zone = napa.zone.current;
```
### <a name="node"></a>node: Zone
It returns a reference to the node zone. It is equivalent to `napa.zone.get('node')`;

Example:
```ts
let zone = napa.zone.node;
```
## <a name="zoneSettings"></a>Interface `ZoneSettings`
Settings for zones, which will  be specified during creation of zones. If not specified, [DEFAULT_SETTINGS](#defaultSettings) will be used.

### <a name="settings-workers"></a>settings.workers: number
Number of workers in the zone.

## <a name="defaultSettings"></a>DEFAULT_SETTINGS
Default settings for creating zones.
```ts
{
    workers: 2
}
```
## <a name="zone"></a>Interface `Zone`
Zone is the basic concept to execute JavaScript and apply policies in Napa. You can find its definition in [Architecture](../../README.md#Architecture). Through Zone API, developers can broadcast JavaScript code on all workers, or execute a function on one of them. When you program against a zone, it is the best practice to ensure all workers within a zone are symmetrical to each other, that you should not assume a worker may maintain its own states.

The two major set of APIs are `broadcast` and `execute`, with both synchronous and asynchronous version plus a few variations on their inputs.
### <a name="zone-id"></a>zone.id: string
It gets the id of the zone.

### <a name="broadcast-code"></a>zone.broadcast(code: string): Promise\<void\>
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
### <a name="broadcast-function"></a>zone.broadcast(function: (...args: any[]) => void, args: any[]): Promise\<void\>
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
### <a name="broadcastSync-code"></a>zone.broadcastSync(code: string): void
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
### <a name="broadcastSync-function"></a>zone.broadcastSync(function: (...args: any[]) => void, args: any[]): void

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
### <a name="execute-name"></a>zone.execute(moduleName: string, functionName: string, args: any[], timeout: number = 0): Promise\<any\>
Execute a function asynchronously on arbitrary worker via module name and function name. Arguments can be of any JavaScript type that is [transportable](transport.md#transportable). It returns a Promise of [`ExecuteResult`](#executeResult). If error happens, either bad code, user exception, or timeout is reached, promise will be rejected.

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

### <a name="execute-function"></a>zone.execute(function: (...args: any[]) => any, args: any[], timeout: number = 0): Promise\<any\>

Execute an anonymous function asynchronously on arbitrary worker. Arguments can be of any JavaScript type that is [transportable](transport.md#transportable). It returns a Promise of [`ExecuteResult`](#executeResult). If error happens, either bad code, user exception, or timeout is reached, promise will be rejected.

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

### <a name="executeSync-name"></a>zone.executeSync(moduleName: string, functionName: string, args: any[], timeout: number = 0): any

Execute a function synchronously on arbitrary worker via module name and function name. Arguments can be of any JavaScript type that is [transportable](transport.md#transportable). It returns an [`ExecuteResult`](#executeResult). If error happens, either bad code, user exception, or timeout is reached, error will be thrown.

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

### <a name="executeSync-function"></a>zone.executeSync(function: (...args: any[]) => any, args: any[], timeout: number = 0): any
Execute an annoymouse function synchronously on arbitrary worker. Arguments can be of any JavaScript type that is [transportable](transport.md#transportable). It returns an [`ExecuteResult`](#executeResult). If error happens, either bad code, user exception, or timeout is reached, error will be thrown.

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

## <a name="executeResult"></a>Interface `ExecuteResult`
Interface to access return value of `zone.execute` or `zone.executeSync`.

### <a name="result-value"></a>result.value: any
JavaScript value returned from function which is invoked from zone.execute/executeSync. Napa marshall/unmarshall [transportable values](transport.md#transportable) between different workers (V8 isolates). Unmarshalling will happen when the first `result.value` is queried.

Example:
```ts
let value = result.value;
```

### <a name="result-payload"></a>result.payload: string
Marshalled payload (in JSON) from returned value. This field is for users that want to pass result through to its caller, where unmarshalled value is not required.  

Example:
```ts
let payload = result.payload;
```

### <a name="result-transportContext"></a>result.transportContext: transport.TransportContext
[TransportContext](transport.md#transportContext) that is required to unmarshall [`result.payload`](#result-payload) into [`result.value`](#result-value).

Example:
```ts
import * as napa from 'napajs';

// Get result from zone.execute...

let transportContext = result.transportContext;
let value = napa.transport.unmarshall(result.payload, result.transportContext);
assert.equal(value, result.value);
```