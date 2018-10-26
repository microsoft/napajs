# Asynchronous numbering

## APIs

Napa provides two functions to support asynchronous call and they work for both node.js and Napa.

```cpp
void napa::module::PostAsyncWork(v8::Local<v8::Function> jsCallback,
                                 std::function<void*()> asyncWork,
                                 std::function<void(v8::Local<v8::Function>, void*)> asyncCompleteCallback);
```

It fires asynchronous work onto separate thread.

* *jsCallback*: Javascript callback given at Javascript land.
* *asyncWork*: C++ function to run at separate thread asynchronously.
* *asyncCompleteCallback*: C++ callback function, which has isolate instance and *jsCallback* as arguments. It's called at the same isolate with caller's one after *asyncWork* completes.

```cpp
void napa::module::DoAsyncWork(v8::Local<v8::Function> jsCallback,
                               const std::function<void(std::function<void(void*)>)>& asyncWork,
                               std::function<void(v8::Local<v8::Function>, void*)> asyncCompleteCallback);
```

If you have a function already supporting async, use this API.

* *jsCallback*: Javascript callback given at Javascript land.
* *asyncWork*: Function to wrap C++ function supporting async, which callback must call Napa completion callback given as argument.
* *asyncCompleteCallback*: C++ callback function, which has isolate instance and *jsCallback* as arguments. It's called at the same isolate with caller's one after *asyncWork* completes.

### Diagram

This diagram shows how asynchronous call is working corresponding to the below example,

```diagram
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
```

## C++ module

This example shows how to create async module. It keeps one number and three APIs operating on it as follows,

* *Increase*: It increases a number by a given parameter in separate thread and post a completion to run Javascript callback at the next execution loop.
* *IncreaseSync*: It increases a number by a given parameter in the same thread and post a completion to run Javascript callback at the next execution loop.
* *Now*: It returns the current value of a number.

```cpp
#include <napa/module.h>

#include <atomic>
#include <functional>

namespace napa {
namespace demo {

using namespace v8;

namespace {
    std::atomic<uint32_t> _now(0);
}

/// <summary> It increases a number by a given parameter asynchronously and runs a callback at the next execution loop. </summary>
void Increase(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();

    CHECK_ARG(isolate,
        args.Length() == 2 && args[0]->IsUint32() && args[1]->IsFunction(),
        "It requires unsigned integer and callback as arguments");

    auto value = args[0]->Uint32Value();

    napa::module::PostAsyncWork(Local<Function>::Cast(args[1]),
        [value]() {
            // This runs at the separate thread.
            _now += value;
            return reinterpret_cast<void*>(static_cast<uintptr_t>(_now.load()));
        },
        [](auto jsCallback, void* result) {
            // This runs at the same thread as one Increase() is called.
            auto isolate = Isolate::GetCurrent();

            int32_t argc = 1;
            Local<Value> argv[] =
                { Integer::NewFromUnsigned(isolate, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(result))) };

            jsCallback->Call(isolate->GetCurrentContext()->Global(), argc, argv);
        }
    );
}

/// <summary> It increases a number by a given parameter synchronously and runs a callback at the next execution loop. </summary>
void IncreaseSync(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();

    CHECK_ARG(isolate,
        args.Length() == 2 && args[0]->IsUint32() && args[1]->IsFunction(),
        "It requires unsigned integer and callback as arguments");

    auto value = args[0]->Uint32Value();

    napa::module::DoAsyncWork(Local<Function>::Cast(args[1]),
        [value](auto complete) {
            // This runs at the same thread.
            _now += value;
            complete(reinterpret_cast<void*>(static_cast<uintptr_t>(_now.load())));
        },
        [](auto jsCallback, void* result) {
            // This runs at the same thread as one IncreaseSync() is called.
            auto isolate = Isolate::GetCurrent();

            int32_t argc = 1;
            Local<Value> argv[] =
                { Integer::NewFromUnsigned(isolate, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(result))) };

            jsCallback->Call(isolate->GetCurrentContext()->Global(), argc, argv);
        }
    );
}

/// <summary> It returns the current value of a number. </summary>
void Now(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();
    HandleScope scope(isolate);

    args.GetReturnValue().Set(Integer::NewFromUnsigned(isolate, _now));
}

void Init(Local<Object> exports) {
    NAPA_SET_METHOD(exports, "increase", Increase);
    NAPA_SET_METHOD(exports, "increaseSync", IncreaseSync);
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

export function increase(value: number, callback: (now: number) => void) {
    return addon.increase(value, callback);
}

export function increaseSync(value: number, callback: (now: number) => void) {
    return addon.increaseSync(value, callback);
}

export function now(): string {
    return addon.now();
}
```

### async-number.d.ts
```d.ts
export declare function increase(extra: number, callback: (now: number) => void): any;
export declare function increaseSync(extra: number, callback: (now: number) => void): any;
export declare function now(): string;
```

## Test

```ts
var assert = require('assert');
var asyncNumber = require('async-number');

describe('Test suite for async-number', function() {
    it('change number asynchronously on separate thread', function(done) {
        let now = asyncNumber.now();
        assert.equal(now, 0);

        asyncNumber.increase(3, (value: number) => {
            // This must be called after the last statement of *it* block is executed.
            assert(value == 3 || value == 6);

            now = asyncNumber.now();
            assert.equal(now, 6);

            done();
        });

        asyncNumber.increaseSync(3, (value) => {} );
    });

    it('change number synchronously on current thread', function(done) {
        let now = asyncNumber.now();
        assert.equal(now, 0);

        asyncNumber.increaseSync(3, (value: number) => {
            // This must be called after the last statement of *it* block is executed.
            assert.equal(value, 3);

            now = asyncNumber.now();
            assert.equal(now, 6);

            done();
        });

        now = asyncNumber.now();
        // 'now' should be 3.
        assert.equal(now, 3);

        asyncNumber.increaseSync(3, (value) => {} );
    });
})
```
