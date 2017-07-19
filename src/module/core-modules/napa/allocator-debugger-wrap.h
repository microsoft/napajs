// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "allocator-wrap.h"

#include <napa/module.h>
#include <napa/memory/allocator-debugger.h>

namespace napa {
namespace module {
    
    /// <summary> It wraps napa::memory::AllocatorDebugger. </summary>
    /// <remarks> Reference: napajs/lib/memory/allocator.ts#AllocatorDebugger </remarks>
    class AllocatorDebuggerWrap: public AllocatorWrap {
    public:
        /// <summary> Init this wrap. </summary>
        static void Init();

        /// <summary> Declare constructor in public, so we can export class constructor to JavaScript world. </summary>
        NAPA_DECLARE_PERSISTENT_CONSTRUCTOR

        /// <summary> Exported class name. </summary>
        static constexpr const char* exportName = "AllocatorDebuggerWrap";

    private:
        /// <summary> Constructor. </summary>
        explicit AllocatorDebuggerWrap(std::shared_ptr<napa::memory::AllocatorDebugger> allocatorDebugger);

        /// <summary> No copy allowed. </summary>
        AllocatorDebuggerWrap(const AllocatorDebuggerWrap&) = delete;
        AllocatorDebuggerWrap& operator=(const AllocatorDebuggerWrap&) = delete;

        /// <summary> AllocatorDebuggerWrap.constructor </summary>
        static void ConstructorCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> It implements AllocatorDebugger.debugInfo </summary>
        static void GetDebugInfoCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    };
}
}
