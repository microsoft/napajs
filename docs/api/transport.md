# namespace `transport`

## Table of Contents
- [Transporting JavaScript values](#transporting-javascript-values)
    - [Transportable types](#transportable-types)
    - [Constructor ID](#constructor-id)
    - [Transport context](#transport-context)
- API
    - [`isTransportable(jsValue: any): boolean`](#istransportable-jsvalue-any)
    - [`register(transportableClass: new(...args: any[]) => any): void`](#register-transportableclass-new-args-any-any-void)
    - [`marshallTransform(jsValue: any, context: TransportContext): any`](#marshalltransform-jsvalue-any-context-transportcontext-any) (unstable)
    - [`marshall(jsValue: any, context: TransportContext): string`](#marshall-jsvalue-any-context-transportcontext-string)
    - [`unmarshall(json: string, context: TransporteContext): any`](#unmarshall-json-string-context-transportecontext-any)
    - class [`TransportContext`](#class-transportcontext)
        - [`context.saveShared(object: memory.Shareable): void`](context-saveshared-object-memory-shareable-void)
        - [`context.loadShared(handle: memory.Handle): memory.Shareable`](context-loadshared-handle-memory-handle-shareable)
        - [`context.sharedCount: number`](context-sharedcount-number)
    - interface [`Transportable`](#interface-transportable)
        - [`transportable.cid: string`](#transportable-cid-string)
        - [`transportable.marshall(context: TransportContext): object`](#transportable-marshall-context-transportcontext-object)
        - [`transportable.unmarshall(payload: object, context: TransportContext): void`](#transportable-unmarshall-payload-object-context-transportcontext-void)
    - abstract class [`TransportableObject`](#abstract-class-transportableobject)
        - [`transportableObject.cid: string`](#transportableobject-cid-string)
        - [`transportableObject.marshall(context: TransportContext): object`](#transportableobject-marshall-context-transportcontext-object)
        - [`transportableObject.unmarshall(payload: object, context: TransportContext): void`](#transportableobject-unmarshall-payload-object-context-transportcontext-void)
        - abstract [`transportableObject.save(payload: object, context: TransportContext): void`](#abstract-transportableobject-save-payload-object-context-transportcontext-void)
        - abstract [`transportableObject.load(payload: object, context: TransportContext): void`](#abstract-transportableobject-load-payload-object-context-transportcontext-void)
    - class [`AutoTransportable`](#class-autotransportable)
    - decorator [`cid`](#decorator-cid)
- [Implementing user transportable classes](#implementing-user-transportable-classes)

## Transporting JavaScript values
Existing JavaScript engines are not designed for running JavaScript across multiple VMs, which means every VM manages their own heap. Passing values from one VM to another has to be marshalled/unmarshalled. The size of payload and complexity of object will greatly impact communication efficiency. In Napa, we try to work out a design pattern for efficient object sharing, based on the fact that all JavaScript VMs (exposed as workers) reside in the same process, and native objects can be wrapped and exposed as JavaScripts objects.

Following concepts are introduced to implement this pattern:

### Transportable types
Transportable types are JavaScript types that can be passed or shared transparently across Napa workers. They are used as value types for passing arguments in [`zone.broadcast`](zone.md#broadcast-function) / [`zone.execute`](zone.md#execute-function), and sharing objects in  key/value pairs via [`store.set`](store.md#store-set-key-string-value-any) / [`store.get`](store.md#store-get-key-string-any).

Transportable types are:
- JavaScript primitive types: undefined, null, boolean, number, string
- Object (TypeScript class) that implement [`Transportable`](#interface-transportable) interface
- Array or plain JavaScript object that is composite pattern of above.

### Constructor ID (cid)
For user classes that implement [`Transportable`](#interface-transportable) interface, Napa uses Constructor ID (`cid`) to lookup constructors for creating a right object from a string payload. `cid` is marshalled as a part of the payload. During unmarshalling, transport layer will extract the `cid`, create an object instance using the constructor associated with it, and then call unmarshall on the object. 

It's class developer's responsibility to choose the right `cid` for your class. To avoid conflict, we suggest to use the combination of module.id and class name as `cid`. Developer can use class decorator [`cid`](#decorator-cid) to register a user Transportable class automatically, when using TypeScript with decorator feature enabled. Or call [`transport.register`](#register-transportableclass-new-args-any-any-void) manually during module initialization.

### Transport context
There are states that cannot be saved or loaded in serialized form (like std::shared_ptr), or it's very inefficient to serialize (like JavaScript function). Transport context is introduced to help in these scenarios. TransportContext objects can be passed from one JavaScript VM to another, or stored in native world, so lifecycle of shared native objects extended by using TransportContext. An example of `Transportable` implementation using TransportContext is [`ShareableWrap`](..\..\inc\napa\module\shareable-wrap.h).

## API

### isTransportable(jsValue: any): boolean
It tells whether a JavaScript value is transportable or not.
```ts
// JS primitives
assert(transport.isTransportable(undefined));
assert(transport.isTransportable(null));
assert(transport.isTransportable(1));
assert(transport.isTransportable('string'));
assert(transport.isTransportable(true));

// Transportable JS object.
@transport.cid(module.id)
class A extends transport.AutoTransportable {
    field1: number;
    field2: string;
}

assert(transport.isTransportable(new A()));

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
### register(transportableClass: new(...args: any[]) => any): void
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
### marshallTransform(jsValue: any, context: TransportContext): any
*unstable - may change in future iterations

Performing marshalling transform on an arbitary JavaScript value, if input is transportable, a plain JavaScript value will be returned. Otherwise an error will be thrown. 

`marshallTransform` is useful when you implement your own transportable class. You can perform `marshallTransform` on member variables to get marshalled form to fill the payload.

Example:
```ts
class SomeClass extends transport.TransportableObject {
    save(payload: object, context: TransportContext) {
        for (let property of Object.getOwnPropertyNames(this)) {
            (<any>(payload))[property] = 
                transport.marshallTransform(
                    (<any>(this))[property], 
                    context);
        }
    }
    //...
}
```
### marshall(jsValue: any, context: TransportContext): string
Marshall a [transportable](#transportable-types) JavaScript value into a JSON payload with a [`TransportContext`](#transport-context). Error will be thrown if the value is not transportable. 

Example:
```ts
let context = transport.createTransportContext();
let jsonPayload = transport.marshall([1, 'string', napa.memory.crtAllocator], context);
console.log(jsonPayload);
```
### unmarshall(json: string, context: TransportContext): any
Unmarshall an [transportable](#transportable-types) JavaScript value from a JSON payload with a [`TransportContext`](#transport-context). Error will be thrown if `cid` property is found and not registered with transport layer.

Example:
```ts
let value = transport.unmarshall(jsonPayload, context);
```

## Class `TransportContext`
Class for [Transport Context](#transport-context), that stores shared pointers and functions during marshall/unmarshall.
### context.saveShared(object: memory.Shareable): void
Save a shareable object in context.

### context.loadShared(handle: memory.Handle): memory.Shareable
Load a shareable object from handle.

### context.sharedCount: number
Count of shareable objects saved in current context.

## Interface `Transportable`
Interface for Transportable object.
### transportable.cid: string
Get accessor for [Constructor ID](#constructor-id). It is used to lookup constructor for payload of current class.

### transportable.marshall(context: TransportContext): object
Marshall transform this object into a plain JavaScript object with the help of [TransportContext](#transport-context).

### transportable.unmarshall(payload: object, context: TransportContext): void
Unmarshall transform marshalled payload into current object.

## Abstract class `TransportableObject`
### Decorator `cid`

## Implementing User Transportable Classes
### JavaScript class
#### Extending TransportableObject
### Addon class
#### Extending ShareableWrap (C++)
#### Write your own