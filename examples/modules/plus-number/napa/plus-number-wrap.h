// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/module.h>
#include <plus-number.h>

namespace napa {
namespace demo {

    /// <summary> Napa example module wrapping PlusNumber class. </summary>
    class PlusNumberWrap : public NAPA_OBJECTWRAP {
    public:

        /// <summary> Register this class into V8. </summary>
        static void Init();

        /// <summary> Enable to create an instance by createPlusNumber() Javascript API. </summary>
        /// <param name="args"> Addend as PlusNumber constructor parameter. </param>
        static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args);

    private:

        /// <summary> Exported class name. </summary>
        static const char* _exportName;

        /// <summary> Constructor with initial value. </summary>
        explicit PlusNumberWrap(double value = 0.0);

        /// <summary> Create PlusNumber instance at V8. </summary>
        /// <param name="args"> Addend as PlusNumber constructor parameter. </param>
        static void NewCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> Add value. </summary>
        /// <param name="args"> Addend. </param>
        static void Add(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> Declare persistent constructor to create PlusNumber instance. </summary>
        /// <remarks> Napa creates persistent constructor at each isolate while node.js creates the static instance. </remarks>
        NAPA_DECLARE_PERSISTENT_CONSTRUCTOR();

        PlusNumber _plusNumber;
    };

}  // namespace demo
}  // namespace napa
