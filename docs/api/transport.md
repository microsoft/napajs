# Namespace `transport`

## Table of Contents
- [Introduction](#intro)
    - [Transportable types](#transportable-types)
    - [Constructor ID](#constructor-id)
    - [Transport context](#transport-context)
    - [Transporting functions](#transporting-functions)
    - [Transporting JavaScript built-in objects](#transporting-built-in)
- API
    - [`isTransportable(jsValue: any): boolean`](#istransportable)
    - [`register(transportableClass: new(...args: any[]) => any): void`](#register)
    - [`marshall(jsValue: any, context: TransportContext): string`](#marshall)
    - [`unmarshall(json: string, context: TransporteContext): any`](#unmarshall)
    - class [`TransportContext`](#transportcontext)
        - [`context.saveShared(object: memory.Shareable): void`](transportcontext-saveshared)
        - [`context.loadShared(handle: memory.Handle): memory.Shareable`](transportcontext-loadshared)
        - [`context.sharedCount: number`](transportcontext-sharedcount)
    - interface [`Transportable`](#transportable)
        - [`transportable.cid: string`](#transportable-cid)
        - [`transportable.marshall(context: TransportContext): object`](#transportable-marshall)
        - [`transportable.unmarshall(payload: object, context: TransportContext): void`](#transportable-unmarshall)
    - abstract class [`TransportableObject`](#transportableobject)
        - [`transportableObject.cid: string`](#transportableobject-cid)
        - [`transportableObject.marshall(context: TransportContext): object`](#transportableobject-marshall)
        - [`transportableObject.unmarshall(payload: object, context: TransportContext): void`](#transportableobject-unmarshall)
        - abstract [`transportableObject.save(payload: object, context: TransportContext): void`](#transportableobject-save)
        - abstract [`transportableObject.load(payload: object, context: TransportContext): void`](#transportableobject-load)
    - decorator [`cid`](#decorator-cid)

## <a name="intro"></a> Introduction
Existing JavaScript engines are not designed for running JavaScript across multiple VMs, which means every VM manages their own heap. Passing values from one VM to another has to be marshalled/unmarshalled. The size of payload and complexity of object will greatly impact communication efficiency. In Napa, we try to work out a design pattern for efficient object sharing, based on the fact that all JavaScript VMs (exposed as workers) reside in the same process, and native objects can be wrapped and exposed as JavaScripts objects.

Following concepts are introduced to implement this pattern:

### <a name="transportable-types"></a>  Transportable types
Transportable types are JavaScript types that can be passed or shared transparently across Napa workers. They are used as value types for passing arguments in [`zone.broadcast`](zone.md#broadcast-function) / [`zone.execute`](zone.md#execute-anonymous-function), and sharing objects in  key/value pairs via [`store.set`](store.md#store-set) / [`store.get`](store.md#store-get).

Transportable types are:
- JavaScript primitive types: undefined, null, boolean, number, string
- Object (TypeScript class) that implement [`Transportable`](#transportable) interface
- Function without referencing closures.
- <a name="built-in-whitelist"></a> JavaScript standard built-In objects in this whitelist.
    * ArrayBuffer
    * Float32Array
    * Float64Array
    * Int16Array
    * Int32Array
    * Int8Array
    * SharedArrayBuffer
    * Uint16Array
    * Uint32Array
    * Uint8Array
- Array or plain JavaScript object that is composite pattern of above.

### <a name="constructor-id"></a> Constructor ID (cid)
For user classes that implement [`Transportable`](#transportable) interface, Napa uses Constructor ID (`cid`) to lookup constructors for creating a right object from a string payload. `cid` is marshalled as a part of the payload. During unmarshalling, transport layer will extract the `cid`, create an object instance using the constructor associated with it, and then call unmarshall on the object. 

It's class developer's responsibility to choose the right `cid` for your class. To avoid conflict, we suggest to use the combination of module.id and class name as `cid`. Developer can use class decorator [`cid`](#decorator-cid) to register a user Transportable class automatically, when using TypeScript with decorator feature enabled. Or call [`transport.register`](#register) manually during module initialization.

### <a name="transport-context"></a> Transport context
There are states that cannot be saved or loaded in serialized form (like std::shared_ptr), or it's very inefficient to serialize (like JavaScript function). Transport context is introduced to help in these scenarios. TransportContext objects can be passed from one JavaScript VM to another, or stored in native world, so lifecycle of shared native objects extended by using TransportContext. An example of `Transportable` implementation using TransportContext is [`ShareableWrap`](./../../inc/napa/module/shareable-wrap.h).

### <a name="transporting-functions"></a> Transporting functions
JavaScript function is a special transportable type, through marshalling its definition into a [store](./store.md#intro), and generate a new function from its definition on target thread. 

Highlights on transporting functions are:
- For the same function, marshall/unmarshall is an one-time cost on each JavaScript thread. Once a function is transported for the first time, later transportation of the same function to previous JavaScript thread can be regarded as free.
- Closure cannot be transported, but you won't get error when transporting a function. Instead, you will get runtime error complaining a variable (from closure) is undefined when you can the function later.
- `__dirname` / `__filename` can be accessed in transported function, which is determined by `origin` property of function. By default `origin` property is set to current working directory.

### <a name="transporting-built-in"></a> Transporting JavaScript built-in objects
JavaScript standard built-In objects in [the whitelist](#built-in-whitelist) can be transported among napa workers transparently. JavaScript Objects with properties in these types are also able to be transported. Please refer to [unit tests](./../../test/transport-test.ts) for detail.

An example [Parallel Quick Sort](./../../examples/tutorial/parallel-quick-sort) demonstrated transporting TypedArray (created from SharedArrayBuffer) among multiple Napa workers for efficient data sharing.

## <a name="api"></a> API

### <a name="istransportable"></a> isTransportable(jsValue: any): boolean
It tells whether a JavaScript value is transportable or not.
```ts
// JS primitives
assert(transport.isTransportable(undefined));
assert(transport.isTransportable(null));
assert(transport.isTransportable(1));
assert(transport.isTransportable('string'));
assert(transport.isTransportable(true));

// Transportable addon
assert(transport.isTransportable(napa.memory.crtAllocator));

// Composite of transportable types.
assert(transport.isTransportable([
    1, 
    "string", 
    { a: napa.memory.crtAllocator }
    ]));

class B {
    field1: number;
    field2: string;
}

// Not transportable JS class. (not registered with @cid).
assert(!transport.isTransportable(new B()));
```
### <a name="register"></a> register(transportableClass: new(...args: any[]) => any): void
Register a `Transportable` class before transport layer can marshall/unmarshall its instances.
User can also use class decorator [`@cid`](#cid-decorator) for class registration.

Example:
```ts
class A extends transport.AutoTransportable {
    field1: string,
    method1(): string {
        return this.field1;
    }
}

// Explicitly register class A in transport.
transport.register(A);
```
### <a name="marshall"></a> marshall(jsValue: any, context: TransportContext): string
Marshall a [transportable](#transportable-types) JavaScript value into a JSON payload with a [`TransportContext`](#transport-context). Error will be thrown if the value is not transportable. 

Example:
```js
var context = transport.createTransportContext();
var jsonPayload = transport.marshall(
    [1, 'string', napa.memory.crtAllocator], 
    context);
console.log(jsonPayload);
```
### <a name="unmarshall"></a> unmarshall(json: string, context: TransportContext): any
Unmarshall an [transportable](#transportable-types) JavaScript value from a JSON payload with a [`TransportContext`](#transport-context). Error will be thrown if `cid` property is found and not registered with transport layer.

Example:
```js
var value = transport.unmarshall(jsonPayload, context);
```

## <a name="transportcontext"></a> Class `TransportContext`
Class for [Transport Context](#transport-context), that stores shared pointers and functions during marshall/unmarshall.
### <a name="transportcontext-saveshared"></a> context.saveShared(object: memory.Shareable): void
Save a shareable object in context.

### <a name="transportcontext-loadshared"></a> context.loadShared(handle: memory.Handle): memory.Shareable
Load a shareable object from handle.

### <a name="transportcontext-sharedcount"></a> context.sharedCount: number
Count of shareable objects saved in current context.

## <a name="transportable"></a> Interface `Transportable`
Interface for Transportable object.
### <a name="transportable-cid"></a> transportable.cid: string
Get accessor for [Constructor ID](#constructor-id). It is used to lookup constructor for payload of current class.

### <a name="transportable-marshall"></a> transportable.marshall(context: TransportContext): object
Marshall transform this object into a plain JavaScript object with the help of [TransportContext](#transport-context).

### <a name="transportable.unmarshall"></a> transportable.unmarshall(payload: object, context: TransportContext): void
Unmarshall transform marshalled payload into current object.

## <a name="transportableobject"></a> Abstract class `TransportableObject`
TBD
### <a name="decorator-cid"></a> Decorator `cid`
TBD
