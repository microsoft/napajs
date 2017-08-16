// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <v8.h>

namespace napa {
namespace module {

    /// <summary> Napa built-in addon to provide helper APIs for process. </summary>
    namespace process {

        /// <summary> Set process object as global variable of given context. </summary>
        /// <param name="exports"> Object to set module. </param>
        void Init(v8::Local<v8::Object> exports);

    } // End of namespace process
} // End of namespace module
} // End of namespace napa
