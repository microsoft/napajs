// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/module.h>
#include <napa/providers/metric.h>

#include <memory>

namespace napa {
namespace module {

    /// <summary> An object wrap to expose metric APIs. </summary>
    class MetricWrap : public node::ObjectWrap {
    public:

        /// <summary> Initializes the wrap. </summary>
        static void Init();

        /// <summary> Create a new MetricWrap instance that wraps the provided metric. </summary>
        static v8::Local<v8::Object> NewInstance(napa::providers::Metric* metric, uint32_t dimensions);

        /// <summary> Declare persistent constructor to create Metric Javascript wrapper instance. </summary>
        static v8::Persistent<v8::Function> _constructor;

    private:

        /// <summary> The underlying metric. </summary>
        napa::providers::Metric* _metric;

        /// <summary> Exported class name. </summary>
        static constexpr const char* _exportName = "MetricWrap";

        /// <summary> Number of dimensions this metric expects. </summary>
        uint32_t _dimensions;

        /// <summary> Constructor. </summary>
        MetricWrap(napa::providers::Metric* metric, uint32_t dimensions);

        /// <summary> MetricWrap.constructor </summary>
        static void ConstructorCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

        // MetricWrap getters
        static void NameGetter(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& args);
        static void SectionGetter(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& args);

        // MetricWrap methods
        static void Set(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void Increment(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void Decrement(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary>
        ///     Helper method that extracts the dimensions and metric from args and calls the func 
        ///     with these arguments.
        /// </summary>
        template <typename Func>
        static void InvokeWithDimensions(const v8::FunctionCallbackInfo<v8::Value>& args, uint32_t index, Func&& func);
    };
}
}