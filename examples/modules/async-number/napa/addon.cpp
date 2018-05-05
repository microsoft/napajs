// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <napa/module.h>
#include <napa/async.h>

#include <atomic>
#include <functional>

namespace napa {
namespace demo {

using namespace v8;

namespace {
    std::atomic<uint32_t> _now(0);
}

/// <summary> It increases a number by a given parameter asynchronously and run a callback at the next execution loop. </summary>
void Increase(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();

    CHECK_ARG(isolate,
        args.Length() == 2 && args[0]->IsUint32() && args[1]->IsFunction(),
        "It requires unsigned integer and callback as arguments");

    auto value = args[0]->Uint32Value();

    napa::zone::PostAsyncWork(Local<Function>::Cast(args[1]),
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

/// <summary> It increases a number by a given parameter synchronously and run a callback at the next execution loop. </summary>
void IncreaseSync(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();

    CHECK_ARG(isolate,
        args.Length() == 2 && args[0]->IsUint32() && args[1]->IsFunction(),
        "It requires unsigned integer and callback as arguments");

    auto value = args[0]->Uint32Value();

    napa::zone::DoAsyncWork(Local<Function>::Cast(args[1]),
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
    NODE_SET_METHOD(exports, "increase", Increase);
    NODE_SET_METHOD(exports, "increaseSync", IncreaseSync);
    NODE_SET_METHOD(exports, "now", Now);
}

NODE_MODULE(addon, Init)

}  // namespace demo
}  // namespace napa
