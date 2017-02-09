# Plus Number Transporter

This example shows how to pass the object instance to a module. It serializes/deserializes object's pointer and passes to
 a module. This is useful when a host application needs to pass a native object to a module and
 serialization/deserialization cost is too expensive. Napa provides helper APIs to serialize and deserialize an existing
 object or new object. The important thing is that applicaiton owner is responsible for making an object alive until Napa
 finishes processing a request containing a passing object. Napa helps type check, but not lifetime management.

## Serialization APIs
These APIs creates an object holder instance and its pointer value to pass to a module.

### Serialize existing object
```cpp
template <typename T>
static std::pair<uintptr_t, std::unique_ptr<ObjectTransporter>> ObjectTransporter::Serialize(T* object)
```
Since a module doesn't know about object's lifetime, this API is good for a global object. *ObjectTransporter* instance
 must survive during request processing.

### Serialize new object
```cpp
template <typename T>
static std::pair<uintptr_t, std::unique_ptr<ObjectTransporter>> ObjectTransporter::Serialize(std::shared_ptr<T> object)
```
*ObjectTransporter* instance must survive during request processing.

## Deserialization APIs
```cpp
template <typename T>
static std::shared_ptr<T> ObjectTransporter::Deserialize(uintptr_t pointer)
```
It returns a passing object instance after deserializing a pointer value to object holder.

## Create new class
When you creates new class to be serializable, inherit from this class.
```cpp
template <typename T>
class TransportableObject
```
This provides *Serialize()* method as follows,
```cpp
std::pair<uintptr_t, std::unique_ptr<ObjectTransporter>> Serialize()
```

## Passing class

*plus-number-transporter.h* declares ths class with one constructor and one method, *Add()*. It's the subclass of
 *TransportableObject<PlusNumberTransporter>* to enable serialization.

```h
#include <napa/object-transportable.h>

namespace napa {
namespace demo {

    /// <summary> Example class to show how to pass object pointer to a module. </summary>
    class PlusNumberTransporter : public module::TransportableObject<PlusNumberTransporter> {
    public:

        /// <summary> Constructor with initial value. </summary>
        PlusNumberTransporter(double value = 0.0);

        /// <summary> Add the given value and return the result. </summary>
        double Add(double value);

    private:

        double _value;
    };

}  // napespace demo
}  // namespace napa
```

## Module

*addon.cpp* defines a module, which has two methods *createPlusNumberTransporter* and *add*.

```cpp
#include <napa-module.h>
#include <napa/object-transport-v8-helpers.h>
#include <plus-number-transporter.h>

namespace napa {
namespace demo {

using namespace v8;

// Since there is no host to store ObjectTransporter instance, make it globally for test.
std::unique_ptr<module::ObjectTransporter> _objectTransporter;

void CreatePlusNumberTransporter(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();
    HandleScope scope(isolate);

    CHECK_ARG(isolate,
        args.Length() == 0 || args.Length() == 1,
        "Only one or no argument is allowed.");

    if (args.Length() == 1) {
        CHECK_ARG(isolate,
            args[0]->IsNumber(),
            "The first argument must be a number.");
    }

    double value = args[0]->IsUndefined() ? 0.0 : args[0]->NumberValue();
    auto plusNumberTransporter = std::make_shared<PlusNumberTransporter>(value);

    auto transporter = plusNumberTransporter->Serialize();
    _objectTransporter.swap(transporter.second);

    auto result = module::object_transport::UintptrToV8Uint32Array(isolate, transporter.first);
    args.GetReturnValue().Set(result);
}

void Add(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();
    HandleScope scope(isolate);

    CHECK_ARG(isolate,
        args.Length() == 1 || args.Length() == 2,
        "Only one or two arguments are allowed.");

    auto transporter = module::object_transport::V8Uint32ArrayToUintptr(isolate, args[0]);
    CHECK_ARG(isolate,
        transporter.second,
        "The first argument must be a uint32 array.");

    if (args.Length() == 2) {
        CHECK_ARG(isolate,
            args[1]->IsNumber(),
            "The second argument must be a number.");
    }
    double value = args[1]->IsUndefined() ? 0.0 : args[1]->NumberValue();

    auto object = module::ObjectTransporter::Deserialize<PlusNumberTransporter>(transporter.first);
    CHECK_ARG(isolate,
        object != nullptr,
        "Can't deserialize ObjectTransporter containing PlusNumberTransporter instance");

    auto result = object->Add(value);
    args.GetReturnValue().Set(Number::New(isolate, result));
}

void InitAll(Local<Object> exports) {
    NAPA_SET_METHOD(exports, "createPlusNumberTransporter", CreatePlusNumberTransporter);
    NAPA_SET_METHOD(exports, "add", Add);
}

NAPA_MODULE(addon, InitAll);

}  // namespace demo
}  // namespace napa
```
* *createPlusNumberTransporter()* returns the uint32 V8 array representation of pointer value to object holder.
* *add()* has two parameters. The first one is the uint32 V8 array pointing to object holder, which is the return value
 of *createPlusNumberTransporter()* in this example. The second one is an addend. This function deserializes and
 restores a passing object and calls its *Add()* method.

### Napa helper APIs
```h
std::pair<uintptr_t, bool> napa::module::object_transport::V8Uint32ArrayToUintptr(v8::Isolate* isolate, const v8::Local<v8::Value>& source)
```
It converts a uint32 V8 array to uintptr. Return the pair of *{0, false}* if conversion fails.</summary>

```h
v8::Local<v8::Array> napa::module::object_transport::UintptrToV8Uint32Array(v8::Isolate* isolate, uintptr_t source)
```
It converts a uintptr to a uint32 V8 array. </summary>

## Typescript
It's recommended that typescript or typescript definition is provided to let the user know the APIs without
 the source codes and develop Typescript project easily.
### plus-number-transporter.ts
```ts
var addon = require('../bin/addon');

export function createPlusNumberTransporter(value: number): any {
    return addon.createPlusNumberTransporter(value);
}

export function add(transporter: any, value: number): number {
    return addon.add(transporter, value);
}
```
### plus-number.d.ts
```d.ts
export declare function createPlusNumberTransporter(value: number): any;
export declare function add(transporter: any, value: number): number;
```

## Mocha test
```js
var assert = require('assert');
var plusNumberTransporter = require('plus-number-transporter');

describe('Test suite for plus-number-transporter', function() {
    it('adds a given value', function() {
        var transporter: any = plusNumberTransporter.createPlusNumberTransporter(3);
        var result: number = plusNumberTransporter.add(transporter, 4);
        assert.equal(result, 7);
    });
})
```