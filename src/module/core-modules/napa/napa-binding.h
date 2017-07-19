// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/module.h>

namespace napa {
namespace module {
namespace binding {

    /// <summary> Initialize and export napa related functions and object wraps. </summary>
    void Init(v8::Local<v8::Object> exports, v8::Local<v8::Object> module);
}
}
}
