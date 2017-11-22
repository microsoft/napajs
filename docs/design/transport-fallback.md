# transport-fallback

## Incentives
In order to efficiently share objects between JavaScript VMs (napa workers), Napa.js defined the abstraction of 'Transportable'. Besides JavaScript primitive types, an object needs to implement 'Transportable' interface to make it transportable. It means [Javascript standard built-in objects](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects) are not transportable unless wrappers or equivalent implementations for them are implemented by implementing 'Transportable' interface. Developing cost of this is not trivial, and new abstraction layer of wrapper or independent implementation will bring barriers for users to learn and adopt these new stuffs. Moreover, we also need to deal with the interaction between JavaScript standards objects and those napa defined objects.

The incentive of transport-fallback is to provide a solution to make JavaScript built-in objects transportable with requirements listed in the Goals section.

At the first stage, we will focus on the efficient solution to share data between napa workers. Basically, it is about making SharedArrayBuffer / TypedArray / DataView transportable.

## Goals
Make Javascript built-in objects transportable with
- a efficient way to share structured data, like SharedArrayBuffer, among napa workers
- consistent APIs with ECMA standards
- no new abstraction layers for the simplest usage
- the least new concepts for advanced usage
- a scalable solution to make all Javascript built-in objects transportable, avoiding to make them transportable one by one.

## Example
This example shows how SharedArrayBuffer object is transported across multiple napa workers. It will print the TypedArray 'ar' created from the SharedArrayBuffer with its all elements set to 100 by different napa workers. 
```js
var napa = require("napajs");
var zone = napa.zone.create('zone', { workers: 4 });

function foo(sab, id) {
    var ar = new Uint8Array(sab);
    Atomics.store(ar, id, 100);
    return id;
}

function run() {
    var start = Date.now();

    var promises = [];
    var sab = new SharedArrayBuffer(4);
    for (var i = 0; i < 4; i++) {
        promises[i] = zone.execute(foo, [sab, i]);
    }

    return Promise.all(promises).then(values => {
        var ar = new Uint8Array(sab);
        console.log(ar);
    });
}

run();

```

## Solution
Here we just give a high level description of the solution. Its api defination will go to docs/api/transport-fallback, and some key concepts or design details will be filled in this file later.
- V8 provides value serialization solution by ValueSerializer and ValueDeserializer, which is compatible with the HTML structured clone algorithm. It is a horizontal solution to serialize / deserialize JavaScript objects. ValueSerializer::Delegate and ValueDeserializer::Delegate are their inner class. They work as base classes from which users can deprive to customize some special handling of external / shared resources, like memory used by a SharedArrayBuffer object.

- V8SerializedData
> 1. The data structure created by V8TransportHelper::SerializeValue(). It hold external / shared resources of an object, like memory used by a SharedArrayBuffer object.
> 2. Only 1 instance will be generated for each object passed to V8TransportHelper::SerializeValue(). A hash function will be implemented to detect whether the given object has been serialized. If yes, just retrieve its SharedPtr of V8SerializedData from Store (or TransportContext).

- V8SerializedObject and V8SerializedObjectWrap : ShareableWrap
> 1. v8SerializedData: SharedPtrWrap
> 2. constructor(object: any, context: TransportContext)
```c++
void ConstructorCallbackconst v8::FunciontCallbackInfo<v8::Value>& args) {
    auto v8SerializedData = v8TransportHelper.SerializeValue(this, object, context);
    auto v8SerializedObject = Create V8 object, and set its properties, like v8SerializedData.
    args.GetReturnValue().Set(originalObject);
}
```
> 3. getOriginalObject(context: TransportContext) : any
```c++
void GetOriginalObjectCallback(const v8::FunciontCallbackInfo<v8::Value>& args) {
    ...
    auto transportContext = get transport context from args;
    auto thisObject = get this object by args.Holder;
    auto originalObject = v8TransportHelper.DeserializeValue(isolate, thisObject, transportContext);
    args.GetReturnValue().Set(originalObject);
}
```

- V8TransportHelper
> 1. Serializer depriving from ValueSerializer::Delegate
> 2. Deserializer depriving from ValueDeserializer::Delegate
> 3. static std::shared_ptr<V8SerializedData> SerializeValue(Isolate* isolate, Local<value> value, local<value> transpotContext)
> 4. static MaybeLocal<Value> DeserializeValue(Isolate* isolate, std::shared_ptr<V8SerializedData> data, local<value> transportContext);

- Currently, napa relies on Transportable API and a registered constructor to make an object transportable. In [marshallTransform](https://github.com/Microsoft/napajs/blob/master/lib/transport/transport.ts), when a JavaScript object is detected to have a registerd constructor, it will go with the existing napa way to marshall this object with the help of a TransportContext object, otherwise a non-transportable error is thrown.

- Instead of throwing an Error when no registerd constructor is detected (here we can also use a whitelist of verified object type to restrict this transport-fallback solution), the above mentioned V8TransportHelper / V8SerializedObjectWrapper can jump in to help marshall this object.
```js
export function marshallTransform(jsValue: any, context: transportable.TransportContext): any {
     if (jsValue != null && typeof jsValue === 'object' && !Array.isArray(jsValue)) {
        let constructorName = Object.getPrototypeOf(jsValue).constructor.name;
        if (constructorName !== 'Object') {
            if (typeof jsValue['cid'] === 'function') {
                return <transportable.Transportable>(jsValue).marshall(context);
            }
            else if (VerifiedObjectList.hasOwnProperty(constructorName)) {
                return new V8SerializedObject(jsValue, context);
            }
            else {
                throw new Error(`Object type \"${constructorName}\" is not transportable.`);
            }
        }
    }
    return jsValue;
}
```
- The reverse process will be invoked in [unmarshallTransform](https://github.com/Microsoft/napajs/edit/master/lib/transport/transport.ts) if the payload is detected to be a V8SerializedObject.
```js
function unmarshallTransform(payload: any, context: transportable.TransportContext): any {
    if (payload != null && payload._cid !== undefined) {
        let cid = payload._cid;
        if (cid === 'function') {
            return functionTransporter.load(payload.hash);
        }
        let subClass = _registry.get(cid);
        if (subClass == null) {
            throw new Error(`Unrecognized Constructor ID (cid) "${cid}". Please ensure @cid is applied on the class or transport.register is called on the class.`);
        }
        let object = new subClass();
        object.unmarshall(payload, context);
        return object;
    }
    else if (isV8SerializedObject(payload)) {
        return <transport-fallback.V8SerializedObject>(payload).getOriginalObject(context);
    }
    return payload;
}
```


## Constraints
The above solution is based on the serialization / deserialization mechanism of V8. It may have the following constraints.
- Not all [JavaScripts standard built-in objects](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects) are supported by Node (as a dependency of Napa node mode) or V8. We only provide transporting solution for those mature object types of Node and V8.
- Up to present, Node does not explicitly support multiple V8 isolates. There might be inconsistency to transport objects between node zone and napa zones. Extra effort might be required to make it consistent.