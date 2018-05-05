# Plus Number

This example shows the napa module, which wraps C++ objects/classes. Instead of using Javascript *new* operator,
 it uses a factory pattern. i.e.

```
var obj = addon.createPlusNumber();
// instead of
// var obj = new addon.PlusNumber();
``` 

## Wrapped class

*plus-number.h* declares ths class with one constructor and one method, *Add()*.

```h
namespace napa {
namespace demo {

    /// <summary> Example class to show how to create Napa module using a wrapped C++ class. </summary>
    class PlusNumber {
    public:

        /// <summary> Constructor with initial value. </summary>
        explicit PlusNumber(double value = 0.0);

        /// <summary> Add the given value and return the result. </summary>
        double Add(double value);

    private:

        double _value;
    };

}  // namespace demo
}  // namespace napa
```

## Wrapper class

*plus-number-wrap.h* declares the wrapper class inherited from *node::ObjectWrap* as follows,

```h
#include <napa/module.h>
#include <plus-number.h>

namespace napa {
namespace demo {

    /// <summary> Napa example module wrapping PlusNumber class. </summary>
    class PlusNumberWrap : public node::ObjectWrap {
    public:

        /// <summary> Register this class into V8. </summary>
        static void Init();

        /// <summary> Enable to create an instance by createPlusNumber() Javascript API. </summary>
        /// <param name="args"> Addend as PlusNumber constructor parameter. </param>
        static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args);

    private:

        /// <summary> Exported class name. </summary>
        static const char* _exportName;

        /// <summary> Constructor with initial value. </summary>
        explicit PlusNumberWrap(double value = 0.0);

        /// <summary> Create PlusNumber instance at V8. </summary>
        /// <param name="args"> Addend as PlusNumber constructor parameter. </param>
        static void NewCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> Add value. </summary>
        /// <param name="args"> Addend. </param>
        static void Add(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> Declare persistent constructor to create PlusNumber instance. </summary>
        /// <remarks> Napa creates persistent constructor at each isolate while node.js creates the static instance. </remarks>
        static v8::Persistent<v8::Function> _constructor;

        PlusNumber _plusNumber;
    };

}  // namespace demo
}  // namespace napa
```

*plus-number-wrap.cpp* implements each functions as follows,

```cpp
#include "plus-number-wrap.h"

using namespace napa::demo;
using namespace v8;

const char* PlusNumberWrap::_exportName = "PlusNumberWrap";

// Define persistent constructor.
NAPA_DEFINE_PERSISTENT_CONSTRUCTOR(PlusNumberWrap);

PlusNumberWrap::PlusNumberWrap(double value)
    : _plusNumber(value) {
}

void PlusNumberWrap::Init() {
    auto isolate = Isolate::GetCurrent();

    // Prepare constructor template.
    auto functionTemplate = FunctionTemplate::New(isolate, NewCallback);
    functionTemplate->SetClassName(String::NewFromUtf8(isolate, _exportName));
    functionTemplate->InstanceTemplate()->SetInternalFieldCount(1);

    // Set prototype method.
    NODE_SET_PROTOTYPE_METHOD(functionTemplate, "add", Add);

    // Set persistent constructor into V8.
    NAPA_SET_PERSISTENT_CONSTRUCTOR(_exportName, functionTemplate->GetFunction());
}

void PlusNumberWrap::NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    HandleScope scope(isolate);

    const int argc = 1;
    Local<Value> argv[argc] = { args[0] };

    auto constructor = NAPA_GET_PERSISTENT_CONSTRUCTOR(_exportName, PlusNumberWrap);
    auto context = isolate->GetCurrentContext();
    auto instance = constructor->NewInstance(context, argc, argv).ToLocalChecked();

    args.GetReturnValue().Set(instance);
}

void PlusNumberWrap::NewCallback(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();
    HandleScope scope(isolate);

    CHECK_ARG(isolate,
        args.IsConstructCall(),
        "PlusNumberWrap instance must be created by the factory.");

    CHECK_ARG(isolate,
        args.Length() == 0 || args.Length() == 1,
        "Only one or no argument is allowed.");

    if (args.Length() == 1) {
        CHECK_ARG(isolate,
            args[0]->IsNumber(),
            "The first argument must be a number.");
    }

    double value = args[0]->IsUndefined() ? 0.0 : args[0]->NumberValue();

    auto wrap = new PlusNumberWrap(value);
    wrap->Wrap(args.This());

    args.GetReturnValue().Set(args.This());
}

void PlusNumberWrap::Add(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();
    HandleScope scope(isolate);

    CHECK_ARG(isolate,
        args.Length() == 1 && args[0]->IsNumber(),
        "Number must be given as argument.");

    auto wrap = node::ObjectWrap::Unwrap<PlusNumberWrap>(args.Holder());
    auto value = wrap->_plusNumber.Add(args[0]->NumberValue());

    args.GetReturnValue().Set(Number::New(isolate, value));
}
```

*addon.cpp* implements the addon as below,

```cpp
#include "plus-number-wrap.h"

using namespace napa::demo;
using namespace v8;

void CreatePlusNumber(const FunctionCallbackInfo<Value>& args) {
    PlusNumberWrap::NewInstance(args);
}

void InitAll(Local<Object> exports) {
    PlusNumberWrap::Init();

    NODE_SET_METHOD(exports, "createPlusNumber", CreatePlusNumber);
}

NODE_MODULE(addon, InitAll);
```

## Transition from node.js module
* *node::ObjectWrap* is equivalent to *node::ObjectWrap*, so object's lifetime works as Javascript object.
* One big difference is how to handle the persistent constructor. While Node.js has only one thread for Javascript and
 static constructor instance is fine, Napa should support one constructor instance per V8 isolate. These macros help
 constructor instance creation.
  * *NAPA_DECLARE_PERSISTENT_CONSTRUCTOR* declares constructor instance.
  * *NAPA_DEFINE_PERSISTENT_CONSTRUCTOR* defines constructor instance.
  * *NAPA_SET_PERSISTENT_CONSTRUCTOR* makes a local constructor as persistent. Napa stores it into thread local storage
   to allow one instance per V8 isolate.
  * *NAPA_GET_PERSISTENT_CONSTRUCTOR* return stored constructor instance.
* *NODE_SET_PROTOTYPE_METHOD* is equivalent to *NODE_SET_PROTOTYPE_METHOD*, which add a prototype method to Javascript
 object.

## Typescript
*plus-number.ts* doesn't need to fully implement *PlusNumber.add()* since the signature of *PlusNumber* instance returned by
 *addon.createPlusNumber()* is the same.
### plus-number.ts
```ts
var addon = require('../bin/addon');

export declare class PlusNumber {
    public add(value: number): number;
}

export function createPlusNumber(value: number = 0): PlusNumber {
    return addon.createPlusNumber(value);
}
```
### plus-number.d.ts
```d.ts
export declare class PlusNumber {
    add(value: number): number;
}
export declare function createPlusNumber(value?: number): PlusNumber;
```

## NPM Package
NPM package contains the additional binary *plus-number.dll*, which is the shared library for *PlusNumber* class. It's placed
 at *bin* directory, so either Node.js or Napa can resolve the shared object path in the same way as it does for a module.

## Mocha test
```js
var assert = require('assert');
var plusNumber = require('plus-number');
describe('Test suite for plus-number', function () {
    it('adds a given value', function () {
        var po = plusNumber.createPlusNumber(3);
        var result = po.add(4);
        assert.equal(result, 7);
    });
    it('fails with constructor call', function () {
        var failed = false;
        try {
            var po = new plusNumber.PlusNumber();
        }
        catch (error) {
            failed = true;
        }
        assert.equal(failed, true);
    });
});
```
