# Asynchronus numbering

## API

*napa-module.h* provides *NAPA_POST_ASYNC_WORK* macro to fire asynchronous job. It has three arguments.

* *jsCallback*: Javascript callback given at Javascript land.
* *asyncWork*: C++ function to run at separate thread asynchronously.
* *asyncCompleteCallback*: C++ callback function, which has isolate instance and *jsCallback* as arguments. It's called at the same isolate with caller's one after *asyncWork* completes.

This diagram shows how asynchoronous call is working corresponding to the below example,

                |-------------------------|     |------------------------|                   |------------------------------------|     |-------------------|
(V8 thread)-----| Call *increase()* at JS |-----| Run the next statement |-------------------| Run *asyncCompleteCallback* at C++ |-----| Call *jsCallback* |
                |-------------------------|     |------------------------|                   |------------------------------------|     |-------------------|
                            |                                                                                +
                            |                                                                                |
                            |----------|                                                                     |
                                       |                                                                     |
                                       +                                                                     |
                             |------------------|     |--------------------------------------------|         |
                (thread)-----| Run  *asyncWork* |-----| Post a task to run *asyncCompleteCallback* |---------|
                             |------------------|     |--------------------------------------------|

This API works for both node.js and Napa.

## C++ module

This example shows how to create async module. It keeps one number and two APIs operates on it as follows,

* *Increase*: It increases a number by a given parameter in separate thread. When asynchronous work completes, it increases by one again at callback,
  so we can verify that async-complete callback is called after a current context's run is done.
* *Now*: It returns the current value of a number.

```cpp
#include <napa-module.h>

#include <atomic>
#include <functional>

namespace napa {
namespace demo {

using namespace v8;

namespace {
    std::atomic<uint32_t> _now(0);
}

void Increase(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();

    CHECK_ARG(isolate,
        args.Length() == 2 && args[0]->IsUint32() && args[1]->IsFunction(),
        "It requires unsigned integer and callback as arguments");

    auto value = args[0]->Uint32Value();

    NAPA_POST_ASYNC_WORK(
        Local<Function>::Cast(args[1]),
        [value]() {
            // This runs at the separate thread.
            _now += value;
        },
        [](auto jsCallback) {
            // This runs at the same thread as one Increase() is called.
            auto isolate = Isolate::GetCurrent();

            int32_t argc = 1;
            Local<Value> argv[] = { Integer::NewFromUnsigned(isolate, ++_now) };

            jsCallback->Call(isolate->GetCurrentContext()->Global(), argc, argv);
        });
}

void Now(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();
    HandleScope scope(isolate);

    args.GetReturnValue().Set(Integer::NewFromUnsigned(isolate, _now));
}

void Init(Local<Object> exports) {
    NAPA_SET_METHOD(exports, "increase", Increase);
    NAPA_SET_METHOD(exports, "now", Now);
}

NAPA_MODULE(addon, Init)

}  // namespace demo
}  // namespace napa
```

## Typescript

### async-number.ts
```ts
var addon = require('../bin/addon');

export function increase(extra: number, callback: (now: number) => void) {
    return addon.increase(extra, callback);
}

export function now(): string {
    return addon.now();
}
```

### async-number.d.ts
```d.ts
export declare function increase(extra: number, callback: (now: number) => void): any;
export declare function now(): string;
```

## Test

```ts
var assert = require('assert');
var asyncNumber = require('async-number');

describe('Test suite for async-number', function() {
    it('change number asynchronously', function(done) {
        let now = asyncNumber.now();
        assert.equal(now, 0);

        asyncNumber.increase(3, (value: number) => {
            // This must be called after the last statement of *it* block is executed.
            assert.equal(value, 4);
            done();
        });

        // Increase can be done either before or after this call.
        now = asyncNumber.now();
        // 'now' should not 4.
        assert(now == 0 || now == 3);
    });
})
```
