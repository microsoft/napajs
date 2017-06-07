# namespace `transport`

## Table of Contents
- [Transporting JavaScript values](#transport)
- [`isTransportable(jsValue: any): boolean`](#isTransportable)
- [`register(transportableClass: new(...args: any[]) => any): void`](#register)
- [`marshallTransform(jsValue: any, context: TransportContext): any`](#marshallTransform) (unstable)
- [`marshall(jsValue: any, context: TransportContext): string`](#marshall)
- [`unmarshall(json: string, context: TransportableContext): any`](#unmarshall)
- class [`TransportContext`](#transportContext)
    - [`context.saveShared(object: memory.Shareable): void`]
    - [`context.loadShared(handle: memory.Handle): memory.Shareable`]
    - [`context.sharedCount: number`]
- interface [`Transportable`](#transportable)
    - [`transportable.cid: string`](#transportable-cid)
    - [`transportable.marshall(context: TransportContext): object`](#transportable-marshall)
    - [`transportable.unmarshall(payload: object, context: TransportContext): void`](#transportable-unmarshall)
- abstract class [`TransportableObject`](#transportableObject)
    - [`transportableObject.cid: string`](#transportableObject-cid)
    - [`transportableObject.marshall(context: TransportContext): object`](#transportableObject-marshall)
    - [`transportableObject.unmarshall(payload: object, context: TransportContext): void`](#transportableObject-unmarshall)
    - abstract [`transportableObject.save(payload: object, context: TransportContext): void`](#transportableObject-save)
    - abstract [`transportableObject.load(payload: object, context: TransportContext): void`](#transportableObject-load)
- class [`AutoTransportable`](#autoTransportable)
- decorator [`cid`](#cid-decorator)
- [Implementing user transportable classes](#implement)

## <a name="transport"></a>Transporting JavaScript values
Existing JavaScript engines are not designed for running JavaScript across multiple VMs, which means every VM manages their own heap. Passing values from one VM to another has to be marshalled/unmarshalled. The size of payload and complexity of object will greatly impact communication efficiency. In Napa, we try to work out a design pattern for efficient object sharing, based on the fact that all JavaScript VMs (exposed as workers) reside in the same process, and native objects can be wrapped and exposed as JavaScripts objects.

Following concepts are introduced to implement this pattern:

### Transportable types
Transportable types are JavaScript types that can be passed or shared transparently across Napa workers. They are used as value types for passing arguments in [`zone.broadcast`](zone.md#broadcast-function) / [`zone.execute`](zone.md#execute-function), and sharing objects in  key/value pairs via [`store.set`](store.md#store-set) / [`store.get`](store.md#store-get).

Transportable types are:
- JavaScript primitive types: undefined, null, boolean, number, string
- Object (TypeScript class) that implement [`Transportable`](#transportable) interface
- Array or plain JavaScript object that is composite pattern of above.

### <a name="cid"></a>Constructor ID (cid)
For user classes that implement [`Transportable`](#transportable) interface, Napa uses Constructor ID (`cid`) to lookup constructors for creating a right object from a string payload. `cid` is marshalled as a part of the payload. During unmarshalling, transport layer will extract the `cid`, create an object instance using the constructor associated with it, and then call unmarshall on the object. 

It's class developer's responsibility to choose the right `cid` for your class. To avoid conflict, we suggest to use the combination of module.id and class name as `cid`. Developer can use class decorator [`cid`](#cid-decorator) to register a user Transportable class automatically, when using TypeScript with decorator feature enabled. Or call [`transport.register`](#register) manually during module initialization.

### <a name="transportContext"></a>Transport context
There are states that cannot be saved or loaded in serialized form (like std::shared_ptr), or it's very inefficient to serialize (like JavaScript function). Transport context is introduced to help in these scenarios. TransportContext objects can be passed from one JavaScript VM to another, or stored in native world, so lifecycle of shared native objects extended by using TransportContext. An example of `Transportable` implementation using TransportContext is [`ShareableWrap`](..\..\inc\napa\module\shareable-wrap.h).

## API

### <a name="isTransportable"></a>isTransportable(jsValue: any): boolean
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
### <a name="register"></a>register(transportableClass: new(...args: any[]) => any): void
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
### <a name="marshallTransform"></a>marshallTransform(jsValue: any, context: TransportContext): any
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
### <a name="marshall"></a>marshall(jsValue: any, context: TransportContext): string
Marshall a [transportable](#transportable) JavaScript value into a JSON payload with a [`TransportContext`](#transportContext). Error will be thrown if the value is not transportable. 

Example:
```ts
let context = transport.createTransportContext();
let jsonPayload = transport.marshall([1, 'string', napa.memory.crtAllocator], context);
console.log(jsonPayload);
```
### <a name="unmarshall"></a>unmarshall(json: string, context: TransportableContext): any
Unmarshall an [transportable](#transportable) JavaScript value from a JSON payload with a [`TransportContext`](#transportContext). Error will be thrown if `cid` property is found and not registered with transport layer.

Example:
```ts
let value = transport.unmarshall(jsonPayload, context);
```

## <a name="transportContext-class"></a>Class `TransportContext`
Class for [Transport Context](#transportContext), that stores shared pointers and functions during marshall/unmarshall.
### <a name="context-saveShared"></a> context.saveShared(object: memory.Shareable): void
Save a shareable object in context.

### <a name="context-loadShared"></a> context.loadShared(handle: memory.Handle): memory.Shareable
Load a shareable object from handle.

### <a name="context-sharedCount"></a> context.sharedCount: number
Count of shareable objects saved in current context.

## <a name="transportable"></a>Interface `Transportable`
Interface for Transportable object.
### <a name="transportable-cid"></a>transportable.cid: string
Get accessor for [Constructor ID](#cid). It is used to lookup constructor for payload of current class.

### <a name="transportable-marshall"></a>transportable.marshall(context: TransportContext): object
Marshall transform this object into a plain JavaScript object with the help of [TransportContext](#transportContext).

### <a name="transportable-unmarshall"></a>transportable.unmarshall(payload: object, context: TransportContext): void
Unmarshall transform marshalled payload into current object.

## <a name="transportableObject"></a>Abstract class `TransportableObject`
### <a name="cid-decorator"></a>Decorator `cid`

## <a name="implement"></a>Implementing User Transportable Classes
### JavaScript class
#### Extending TransportableObject
### Addon class
#### Extending ShareableWrap (C++)
#### Write your own