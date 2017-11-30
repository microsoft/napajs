// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/module.h>
#include <napa/module/shareable-wrap.h>

namespace napa {
namespace module {

    /// <summary> An object wrap to expose lock APIs. </summary>
    class LockWrap : public ShareableWrap {
    public:

        /// <summary> Initializes the wrap. </summary>
        static void Init();

        /// <summary> Creates a new instance of LockWrap. </summary>
        static v8::Local<v8::Object> NewInstance();

        /// <summary> Exported class name. </summary>
        static constexpr const char* exportName = "LockWrap";

        /// <summary> Declare persistent constructor to create Lock Javascript wrapper instance. </summary>
        NAPA_DECLARE_PERSISTENT_CONSTRUCTOR;

    private:

        // LockWrap methods
        static void GuardSyncCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    };
}
}