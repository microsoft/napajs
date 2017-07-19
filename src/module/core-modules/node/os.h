// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <v8.h>

namespace napa {
namespace module {

/// <summary> Napa built-in addon for operating system. </summary>
namespace os {

    /// <summary> Set os object. </summary>
    /// <param name="exports"> Object to set module. </param>
    void Init(v8::Local<v8::Object> exports);

}   // End of namespace os
}   // End of namespace module
}   // End of namespace napa
