#pragma once

#include <napa-module.h>

#include <memory>
#include <mutex>
#include <string>

namespace napa {
namespace module {

    /// <summary> It holds a shared pointer to native object. </summary>
    class SharedWrap : public NAPA_OBJECTWRAP {
    public:

        /// <summary> It creates a persistent constructor for SharedWrap instance. </summary>
        static void Init() {
            auto isolate = v8::Isolate::GetCurrent();

            auto functionTemplate = v8::FunctionTemplate::New(isolate, NewCallback);
            functionTemplate->SetClassName(v8_helpers::MakeV8String(isolate, _exportName));
            functionTemplate->InstanceTemplate()->SetInternalFieldCount(1);

            NAPA_SET_PERSISTENT_CONSTRUCTOR(_exportName, functionTemplate->GetFunction());
        }

        /// <summary> It creates V8 object holding a given object. </summary>
        /// <param name="object"> Shared object instance. </summary>
        template <typename T>
        static v8::Local<v8::Object> Create(std::shared_ptr<T> object) {
            auto isolate = v8::Isolate::GetCurrent();
            v8::EscapableHandleScope scope(isolate);

            auto instance = NewInstance();
            auto sharedWrap = NAPA_OBJECTWRAP::Unwrap<SharedWrap>(instance);
            sharedWrap->_object = std::static_pointer_cast<void>(std::move(object));

            return scope.Escape(instance);
        }

        /// <summary> Get a shared pointer to a native object. </summary>
        template <typename T = void>
        typename std::enable_if_t<!std::is_same<void, T>::value, std::shared_ptr<T>> Get() {
            return std::static_pointer_cast<T>(_object);
        }

        /// <summary> Get a void shared pointer to a native object. </summary>
        template <typename T = void>
        typename std::enable_if<std::is_same<void, T>::value, std::shared_ptr<void>>::type Get() {
            return _object;
        }

    private:

        /// <summary> Exported class name. </summary>
        static constexpr const char* _exportName = "SharedWrap";

        /// <summary> Create V8 object at Javascript land. </summary>
        static void NewCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
            auto isolate = v8::Isolate::GetCurrent();
            v8::HandleScope scope(isolate);

            NAPA_ASSERT(args.IsConstructCall(), "SharedWrap allows constructor call only.");

            // It's deleted when its Javascript object is garbage collected by V8's GC.
            auto wrap = new SharedWrap();
            wrap->Wrap(args.This());

            args.GetReturnValue().Set(args.This());
        }

        /// <summary> Create V8 object at native land. </summary>
        static v8::Local<v8::Object> NewInstance() {
            auto isolate = v8::Isolate::GetCurrent();
            v8::EscapableHandleScope scope(isolate);

            auto constructor = NAPA_GET_PERSISTENT_CONSTRUCTOR(_exportName);
            return scope.Escape(constructor->NewInstance());
        }

        /// <summary> Default constructor. </summary>
        SharedWrap() = default;

        /// <summary> Constructor. </summary>
        explicit SharedWrap(std::shared_ptr<void> object) : _object(std::move(object)) {}

        /// <summary> Shared object. </summary>
        std::shared_ptr<void> _object;

        NAPA_DECLARE_PERSISTENT_CONSTRUCTOR
    };

/// <summary> It defines the persistent constructor for SharedWrap class. <summary>
/// <remarks> Any module using ShardWrap must call this macro. </remarks>
#define NAPA_DEFINE_PERSISTENT_SHARED_WRAP_CONSTRUCTOR NAPA_DEFINE_PERSISTENT_CONSTRUCTOR(SharedWrap)

/// <summary> It creates and registers the persistent constructor for SharedWrap class. <summary>
/// <remarks> Any module using ShardWrap must run this macro. </remarks>
#define NAPA_SHARED_WRAP_INIT() napa::module::SharedWrap::Init()

}
}