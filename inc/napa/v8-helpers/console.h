// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/v8-helpers/string.h>
#include <napa/v8-helpers/function.h>
#include <v8.h>

namespace napa {
namespace v8_helpers {
    struct Console {
        /// <summary> Call console.log from C++ </summary>
        /// <param name="argc"> Number of arguments. </param>
        /// <param name="argv"> Actual arguments. </param>
        static void Log(int argc, v8::Local<v8::Value> argv[]) {
            auto isolate = v8::Isolate::GetCurrent();
            v8::HandleScope scope(isolate);
            auto context = isolate->GetCurrentContext();

            auto console =
                v8::Local<v8::Object>::Cast(context->Global()->Get(v8_helpers::MakeV8String(isolate, "console")));

            v8_helpers::Call(console, "log", argc, argv);
        }
    };
}
}