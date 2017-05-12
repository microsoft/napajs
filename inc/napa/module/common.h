#pragma once

#include <napa/v8-helpers.h>

namespace napa {
namespace module {

    /// <summary> It defines a default constructor for a NAPA_OBJECTWRAP sub-class from Javascript world. </summary>
    /// <remarks>
    ///     1. const char* WrapType::exportName must be present as a member of WrapType.
    ///     2. DefaultConstructorCallback must be declared as a friend function in WrapType.
    /// </remarks>
    template <typename WrapType>
    inline void DefaultConstructorCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        CHECK_ARG(isolate, args.Length() == 0, "class \"%s\" doesn't accept any arguments in constructor.'", WrapType::exportName);
        JS_ENSURE(isolate, args.IsConstructCall(), "class \"%s\" allows constructor call only.", WrapType::exportName);

        // It's deleted when its Javascript object is garbage collected by V8's GC.
        auto wrap = new WrapType();
        wrap->Wrap(args.This());
        args.GetReturnValue().Set(args.This());
    }

    /// <summary> Create an instance of WrapType with arugments. </summary> 
    /// <remarks> There are 2 requirements on WrapType:
    /// 1) static const char* WrapType::exportName must be present as a public member, or add NewInstance as a friend function.
    /// 2) WrapType must put NAPA_DECLARE_PERSISTENT_CONSTRUCTOR in public, or add NewInstance as a friend function. 
    /// </remarks>
    template <typename WrapType>
    inline v8::MaybeLocal<v8::Object> NewInstance(int argc = 0, v8::Local<v8::Value> argv[] = nullptr) {
        auto constructor = NAPA_GET_PERSISTENT_CONSTRUCTOR(WrapType::exportName, WrapType);
        return constructor->NewInstance(argc, argv);
    }
}
}