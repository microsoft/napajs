// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <napa/module.h>

namespace napa {
namespace demo {

using namespace v8;

void Method(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, "world"));
}

void Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "hello", Method);
}

NODE_MODULE(addon, Init)

}  // namespace demo
}  // namespace napa
