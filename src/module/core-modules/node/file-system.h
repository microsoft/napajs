// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <string>
#include <v8.h>

namespace napa {
namespace module {

/// <summary> Napa built-in addon for file system operations. </summary>
namespace file_system {

    /// <summary> Set file system object. </summary>
    /// <param name="exports"> Object to set module. </param>
    void Init(v8::Local<v8::Object> exports);

} // namespace file_system
} // namespace module
} // namespace napa