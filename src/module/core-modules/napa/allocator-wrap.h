// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/memory/allocator.h>
#include <napa/module.h>
#include <napa/module/shareable-wrap.h>

namespace napa {
namespace module {
    /// <summary> Interface for AllocatorWrap. </summary>
    class AllocatorWrap : public ShareableWrap {
    public:
        /// <summary> It creates a persistent constructor for AllocatorWrap instance. </summary>
        static void Init();

        /// <summary> Get transport context. </summary>
        std::shared_ptr<napa::memory::Allocator> Get();

        /// <summary> Exported class name. </summary>
        static constexpr const char* exportName = "AllocatorWrap";

        /// <summary> Declare constructor in public, so we can export class constructor in JavaScript world. </summary>
        NAPA_DECLARE_PERSISTENT_CONSTRUCTOR

    protected:
        /// <summary> It implements Allocator.allocate(size: number): napajs.memory.Handle </summary>
        static void AllocateCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> It implements Allocator.allocate(size: number): napajs.memory.Handle </summary>
        static void DeallocateCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> It implements readonly Allocator.type: string </summary>
        static void GetTypeCallback(v8::Local<v8::String> propertyName,
                                    const v8::PropertyCallbackInfo<v8::Value>& args);
    };
}
}
