// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

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
