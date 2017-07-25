// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa.h>

#include <atomic>

namespace napa {
namespace test {

    /// <summary> A simple object wrap for testing. </summary>
    class SimpleObjectWrap : public NAPA_OBJECTWRAP {
    public:

        /// <summary> Exported class name. </summary>
        static constexpr const char* exportName = "SimpleObjectWrap";

        /// <summary> Initializes the wrap. </summary>
        static void Init();

        /// <summary> Create a new wrap instance. </summary>
        static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args);

        std::atomic<uint32_t> value = { 0 };

    private:

        /// <summary> Declare persistent constructor to create Zone Javascript wrapper instance. </summary>
        NAPA_DECLARE_PERSISTENT_CONSTRUCTOR;

        // SimpleObjectWrap methods
        static void GetValue(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void SetValue(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void DoIncrementWork(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void PostIncrementWork(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> Friend default constructor callback. </summary>
        template <typename WrapType>
        friend void napa::module::DefaultConstructorCallback(const v8::FunctionCallbackInfo<v8::Value>&);
    };
}
}