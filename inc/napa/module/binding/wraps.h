#pragma once

#include <napa/module/binding.h>
#include <napa/module/shareable-wrap.h>
#include <napa/memory/allocator.h>
#include <napa/memory/allocator-debugger.h>

#include <v8.h>

namespace napa {
namespace module {
namespace binding {
    /// <summary> It creates a new instance of wrapType with a shared_ptr<T>. </summary>
    /// <param name="object"> shared_ptr of object. </summary>
    /// <param name="wrapType"> wrap type from napa-binding, which extends napa::module::Sharable. </param>
    /// <returns> V8 object of wrapType. </summary>
    template <typename T>
    inline v8::Local<v8::Object> CreateShareableWrap(std::shared_ptr<T> object, const char* wrapType = "SharedPtrWrap") {
        auto instance = NewInstance(wrapType, 0, nullptr).ToLocalChecked();
        ShareableWrap::Set(instance, std::move(object));
        return instance;
    }

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