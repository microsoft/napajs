#pragma once

#include <napa-module.h>
#include <napa/module/allocator-wrap.h>

namespace napa {
namespace binding {
    
    /// <summary> It wraps napa::memory::SimpleAllocatorDebugger. </summary>
    /// <remarks> Reference: napajs/lib/memory/allocator.ts#AllocatorDebugger </remarks>
    class SimpleAllocatorDebuggerWrap: public napa::module::AllocatorWrap {
    public:
        /// <summary> Init this wrap. </summary>
        static void Init();

        /// <summary> Declare constructor in public, so we can export class constructor to JavaScript world. </summary>
        NAPA_DECLARE_PERSISTENT_CONSTRUCTOR

        /// <summary> Exported class name. </summary>
        static constexpr const char* exportName = "SimpleAllocatorDebuggerWrap";

    private:
        /// <summary> Constructor. </summary>
        explicit SimpleAllocatorDebuggerWrap(std::shared_ptr<napa::memory::Allocator> allocator);

        /// <summary> No copy allowed. </summary>
        SimpleAllocatorDebuggerWrap(const SimpleAllocatorDebuggerWrap&) = delete;
        SimpleAllocatorDebuggerWrap& operator=(const SimpleAllocatorDebuggerWrap&) = delete;

        /// <summary> SimpleAllocatorDebuggerWrap.constructor </summary>
        static void ConstructorCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> It implements AllocatorDebugger.debugInfo </summary>
        static void GetDebugInfoCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    };
}
}
