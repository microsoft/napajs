#pragma once

#include <napa-module.h>
#include <napa-transport.h>

#include <napa/module/common.h>
#include <napa/module/transport-context-wrap.h>

#include <memory>
#include <string>

namespace napa {
namespace module {

    /// <summary> Abstract class for wraps that contains a C++ std::shared_ptr<T> and allow it be shared across isolates. </summary>
    /// <remarks> see napajs/lib/memory/shareable.ts </remarks>
    class ShareableWrap : public NAPA_OBJECTWRAP {
    public:

        /// <summary> It initialize constructor template of a sub-class of ShareableWrap. </summary>
        /// <param name="constructorTemplate"> Constructor template of wrap class. </param>
        /// <remarks> Should call this method in sub-class Init. </remarks>
        template <typename WrapType>
        static void InitConstructorTemplate(v8::Local<v8::FunctionTemplate> constructorTemplate) {
            // Blessed with methods from napajs.transport.Transportable.
            napa::transport::TransportableObject::InitConstructorTemplate(constructorTemplate);

            NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "load", WrapType::LoadCallback);
            NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "save", WrapType::SaveCallback);
            NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "isNull", WrapType::IsNullCallback);
            NAPA_SET_ACCESSOR(constructorTemplate, "refCount", WrapType::RefCountCallback, nullptr);
            NAPA_SET_ACCESSOR(constructorTemplate, "handle", WrapType::GetHandleCallback, nullptr);
        }

        /// <summary> It initialize constructor of a sub-class of ShareableWrap. </summary>
        /// <remarks> Should call this method in sub-class Init. </remarks>
        /// <param name="cid"> Cid used for transporting the wrap. </param>
        /// <param name='constructor'> Constructor of wrap class. </param>
        static void InitConstructor(const char* cid, v8::Local<v8::Function> constructor) {
            napa::transport::TransportableObject::InitConstructor(cid, constructor);
        }

        /// <summary> Set an instance of ShareableWrap child-class with shared_ptr of T. </summary>
        template <typename T>
        static void Set(v8::Local<v8::Object> wrap, std::shared_ptr<T> object) {
            auto shareable = NAPA_OBJECTWRAP::Unwrap<ShareableWrap>(wrap);
            shareable->_object = std::static_pointer_cast<void>(std::move(object));
        }

        /// <summary> Get shared_ptr of T, which is the type of contained native object. </summary>
        template <typename T = void>
        typename std::enable_if_t<!std::is_same<void, T>::value, std::shared_ptr<T>> Get() {
            return std::static_pointer_cast<T>(_object);
        }

        /// <summary> Get shared_ptr of void to a native object. </summary>
        template <typename T = void>
        typename std::enable_if<std::is_same<void, T>::value, std::shared_ptr<void>>::type Get() {
            return _object;
        }

        /// <summary> It creates a new instance of WrapType of shared_ptr<T>, WrapType is a sub-class of ShareableWrap. </summary>
        /// <param name="object"> shared_ptr of object. </summary>
        /// <returns> V8 object of type ShareableWrap. </summary>
        template <typename T, typename WrapType>
        static v8::Local<v8::Object> NewInstance(std::shared_ptr<T> object) {
            auto instance = napa::module::NewInstance<WrapType>().ToLocalChecked();
            Set(instance, std::move(object));
            return instance;
        }

    protected:
        /// <summary> Friend default constructor callback to access protected method NAPA_OBJECTWRAP::Unwrap. </summary>
        template <typename T>
        friend void napa::module::DefaultConstructorCallback(const v8::FunctionCallbackInfo<v8::Value>&);

        /// <summary> Default constructor. </summary>
        ShareableWrap() = default;

        /// <summary> Constructor. </summary>
        explicit ShareableWrap(std::shared_ptr<void> object) : _object(std::move(object)) {}

        /// <summary> Allow inheritance. </summary>
        virtual ~ShareableWrap() = default;

        /// <summary> It implements readonly Shareable.handle : Handle </summary>
        static void GetHandleCallback(v8::Local<v8::String> /*propertyName*/, const v8::PropertyCallbackInfo<v8::Value>& args){
            auto isolate = v8::Isolate::GetCurrent();
            v8::HandleScope scope(isolate);
            auto thisObject = NAPA_OBJECTWRAP::Unwrap<ShareableWrap>(args.Holder());
            args.GetReturnValue().Set(v8_helpers::PtrToV8Uint32Array(isolate, thisObject->_object.get()));
        }

        /// <summary> It implments Shareable.refCount(): boolean </summary>
        static void RefCountCallback(v8::Local<v8::String> /*propertyName*/, const v8::PropertyCallbackInfo<v8::Value>& args){
            auto isolate = v8::Isolate::GetCurrent();
            v8::HandleScope scope(isolate);
            auto thisObject = NAPA_OBJECTWRAP::Unwrap<ShareableWrap>(args.Holder());
            args.GetReturnValue().Set(static_cast<int32_t>(thisObject->_object.use_count()));
        }

        /// <summary> It implements Shareable.isNull(): boolean </summary>
        static void IsNullCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
            auto isolate = v8::Isolate::GetCurrent();
            v8::HandleScope scope(isolate);
            auto thisObject = NAPA_OBJECTWRAP::Unwrap<ShareableWrap>(args.Holder());
            args.GetReturnValue().Set(thisObject->_object.get() == nullptr);
        }

        /// <summary> It implements TransportableObject.load(payload: object, transportContext: TransportContext): void </summary>
        static void LoadCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
            auto isolate = v8::Isolate::GetCurrent();
            auto context = isolate->GetCurrentContext();
            v8::HandleScope scope(isolate);
            
            CHECK_ARG(isolate, args.Length() == 2, "2 arguments are required for \"load\".");
            CHECK_ARG(isolate, args[0]->IsObject(), "Argument \"payload\" shall be 'Object' type.");
            CHECK_ARG(isolate, args[1]->IsObject(), "Argument \"transportContext\" shall be 'TransportContextWrap' type.");

            auto payload = v8::Local<v8::Object>::Cast(args[0]);
            auto numberArray = payload->Get(v8_helpers::MakeV8String(isolate, "handle"));
            
            auto result = v8_helpers::V8ValueToUintptr(isolate, numberArray);
            JS_ENSURE(isolate, result.second, "Unable to cast \"handle\" to pointer. Please check if it's in valid handle format.");
            
            auto transportContextWrap = NAPA_OBJECTWRAP::Unwrap<TransportContextWrap>(v8::Local<v8::Object>::Cast(args[1]));
            JS_ENSURE(isolate, transportContextWrap != nullptr, "Argument \"transportContext\" should be 'TransportContextWrap' type.");
    
            // Load object from transport context.
            auto thisObject = NAPA_OBJECTWRAP::Unwrap<ShareableWrap>(args.Holder());
            thisObject->_object = transportContextWrap->Get()->LoadShared<void>(result.first);
        }

        /// <summary> It implements TransportableObject.save(payload: object, transportContext: TransportContext): void </summary>
        static void SaveCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
            auto isolate = v8::Isolate::GetCurrent();
            auto context = isolate->GetCurrentContext();
            v8::HandleScope scope(isolate);
            
            CHECK_ARG(isolate, args.Length() == 2, "2 arguments are required for \"save\".");
            CHECK_ARG(isolate, args[0]->IsObject(), "Argument \"payload\" should be 'Object' type.");
            CHECK_ARG(isolate, args[1]->IsObject(), "Argument \"transportContext\" should be 'TransportContextWrap' type.");

            auto payload = v8::Local<v8::Object>::Cast(args[0]);
            auto transportContextWrap = NAPA_OBJECTWRAP::Unwrap<TransportContextWrap>(v8::Local<v8::Object>::Cast(args[1]));
            JS_ENSURE(isolate, transportContextWrap != nullptr, "Argument \"transportContext\" should be 'TransportContextWrap' type.");

            auto thisObject = NAPA_OBJECTWRAP::Unwrap<ShareableWrap>(args.Holder());
            payload->CreateDataProperty(
                context,
                v8_helpers::MakeV8String(isolate, "handle"),
                v8_helpers::PtrToV8Uint32Array(isolate, thisObject->_object.get()));

            // Save object to transport context.
            transportContextWrap->Get()->SaveShared(thisObject->_object);
        }

        /// <summary> Shared object. </summary>
        std::shared_ptr<void> _object;
    };
}
}