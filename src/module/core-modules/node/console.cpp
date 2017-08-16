// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "console.h"

#include <napa/module.h>

#include <iostream>
#include <sstream>

using namespace napa;
using namespace napa::module;

namespace {

/// <summary> Log a message to console. </summary>
/// <param name="args"> All arguments are converted to string and printed out to console. </param>
void LogCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

} // End of anonymous namespace.

void console::Init(v8::Local<v8::Object> exports) {
    NAPA_SET_METHOD(exports, "log", LogCallback);
}

namespace {

void LogCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    std::ostringstream oss;
    for (int i = 0; i < args.Length(); ++i) {
        v8::String::Utf8Value argument(args[i]->ToString());
        oss << *argument << " ";
    }

    std::string message = oss.str();
    if (!message.empty()) {
        message.pop_back();
    }

    std::cout << message << std::endl;

    args.GetReturnValue().Set(args.Holder());
}

} // End of anonymous namespace.