// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/module/binding/basic-wraps.h>
#include <napa/memory/allocator.h>
#include <napa/memory/allocator-debugger.h>

#include <v8.h>

namespace napa {
namespace module {
namespace binding {

    /// <summary> It creates a new instance of AllocatorWrap. </summary>
    /// <param name="allocator"> shared_ptr of allocator. </summary>
    /// <returns> V8 object of AllocatorWrap. </summary>
    inline v8::Local<v8::Object> CreateAllocatorWrap(std::shared_ptr<napa::memory::Allocator> allocator) {
        return CreateShareableWrap(allocator, "AllocatorWrap");
    }

    /// <summary> It creates a new instance of AllocatorDebuggerWrap. </summary>
    /// <param name="allocator"> shared_ptr of allocatorDebugger. </summary>
    /// <returns> V8 object of AllocatorDebuggerWrap. </summary>
    inline v8::Local<v8::Object> CreateAllocatorDebuggerWrap(std::shared_ptr<napa::memory::AllocatorDebugger> allocatorDebugger) {
        return CreateShareableWrap(allocatorDebugger, "AllocatorDebuggerWrap");
    }
}
}
}