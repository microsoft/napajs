#pragma once

#include <napa-module.h>
#include <napa/module/transport.h>

#include <memory>
#include <string>

namespace napa {
namespace module {

    /// <summary> It wraps C++ std::shared_ptr<T> and allow it be shared across isolates. </summary>
    /// <remarks> see napajs/lib/memory/shared-wrap.d.ts#SharedWrap </remarks>
    class SharedWrap : public NAPA_OBJECTWRAP {
    public:
        /// <summary> It creates a persistent constructor for SharedWrap instance. </summary>
        static void Init() {
            auto isolate = v8::Isolate::GetCurrent();
            auto constructorTemplate = v8::FunctionTemplate::New(isolate, ConstructorCallback);
            constructorTemplate->SetClassName(v8_helpers::MakeV8String(isolate, _exportName));
            constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);

            InitConstructorTemplate(constructorTemplate);
            auto constructor = constructorTemplate->GetFunction();
            InitConstructor("<SharedWrap>", constructor);
            NAPA_SET_PERSISTENT_CONSTRUCTOR(_exportName, constructor);
        }

        /// <summary> It initialize constructor template of SharedWrap or sub-class to have SharedWrap methods. </summary>
        /// <param name="constructorTemplate"> Constructor template of wrap class. </param>
        /// <remarks> Should call this method in sub-class Init. </remarks>
        static void InitConstructorTemplate(v8::Local<v8::FunctionTemplate> constructorTemplate) {
            // Blessed with methods from napajs.transport.Transportable.
            TransportableObject::InitConstructorTemplate(constructorTemplate);

            NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "load", LoadCallback);
            NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "save", SaveCallback);
            NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "isNull", IsNullCallback);
            NAPA_SET_ACCESSOR(constructorTemplate, "refCount", RefCountCallback, nullptr);
            NAPA_SET_ACCESSOR(constructorTemplate, "handle", GetHandleCallback, nullptr);
        }

        /// <summary> It initialize constructor of SharedWrap or sub-class. </summary>
        /// <remarks> Should call this method in sub-class Init. </remarks>
        /// <param name="cid"> Cid used for transporting the wrap. </param>
        /// <param name='constructor'> Constructor of wrap class. </param>
        static void InitConstructor(const char* cid, v8::Local<v8::Function> constructor) {
            TransportableObject::InitConstructor(cid, constructor);
        }

        /// <summary> It creates SharedWrap from C++ world. </summary>
        /// <param name="object"> shared_ptr of object. </summary>
        /// <returns> V8 object of type SharedWrap. </summary>
        template <typename T>
        static v8::Local<v8::Object> Create(std::shared_ptr<T> object) {
            auto isolate = v8::Isolate::GetCurrent();
            v8::EscapableHandleScope scope(isolate);

            auto constructor = NAPA_GET_PERSISTENT_CONSTRUCTOR(_exportName, napa::module::SharedWrap);
            auto instance = constructor->NewInstance();
            auto wrap = NAPA_OBJECTWRAP::Unwrap<SharedWrap>(instance);
            wrap->_object = std::static_pointer_cast<void>(std::move(object));

            return scope.Escape(instance);
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

        /// <summary> Declare constructor in public, so we can export class constructor in JavaScript world. </summary>
        NAPA_DECLARE_PERSISTENT_CONSTRUCTOR

    protected:
        /// <summary> Default constructor. </summary>
        SharedWrap() = default;

        /// <summary> Constructor. </summary>
        explicit SharedWrap(std::shared_ptr<void> object) : _object(std::move(object)) {}

    private:
        /// <summary> Exported class name. </summary>
        static constexpr const char* _exportName = "SharedWrap";
        
        /// <summary> It creates SharedWrap from Javascript world. </summary>
        static void ConstructorCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
            auto isolate = v8::Isolate::GetCurrent();
            v8::HandleScope scope(isolate);

            CHECK_ARG(isolate, args.Length() == 0, "class \"SharedWrap\" doesn't accept any arguments in constructor.'");
            JS_ENSURE(isolate, args.IsConstructCall(), "class \"SharedWrap\" allows constructor call only.");

            // It's deleted when its Javascript object is garbage collected by V8's GC.
            auto wrap = new SharedWrap();
            wrap->Wrap(args.This());
            args.GetReturnValue().Set(args.This());
        }

        /// <summary> It implements readonly Shareable.handle : Handle </summary>
        static void GetHandleCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& args){
            auto isolate = v8::Isolate::GetCurrent();
            v8::HandleScope scope(isolate);
            auto thisObject = NAPA_OBJECTWRAP::Unwrap<SharedWrap>(args.Holder());
            args.GetReturnValue().Set(v8_helpers::PtrToV8Uint32Array(isolate, thisObject->_object.get()));
        }

        /// <summary> It implments Shareable.refCount(): boolean </summary>
        static void RefCountCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& args){
            auto isolate = v8::Isolate::GetCurrent();
            v8::HandleScope scope(isolate);
            auto thisObject = NAPA_OBJECTWRAP::Unwrap<SharedWrap>(args.Holder());
            args.GetReturnValue().Set(thisObject->_object.use_count());
        }

        /// <summary> It implements Shareable.isNull(): boolean </summary>
        static void IsNullCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
            auto isolate = v8::Isolate::GetCurrent();
            v8::HandleScope scope(isolate);
            auto thisObject = NAPA_OBJECTWRAP::Unwrap<SharedWrap>(args.Holder());
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
            auto thisObject = NAPA_OBJECTWRAP::Unwrap<SharedWrap>(args.Holder());
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

            auto thisObject = NAPA_OBJECTWRAP::Unwrap<SharedWrap>(args.Holder());
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

/// <summary> It defines the persistent constructor for SharedWrap class. <summary>
/// <remarks> Any module using ShardWrap must call this macro. </remarks>
#define NAPA_DEFINE_PERSISTENT_SHARED_WRAP_CONSTRUCTOR NAPA_DEFINE_PERSISTENT_CONSTRUCTOR(napa::module::SharedWrap)

/// <summary> It creates and registers the persistent constructor for SharedWrap class. <summary>
/// <remarks> Any module using ShardWrap must run this macro. </remarks>
#define NAPA_SHARED_WRAP_INIT() napa::module::SharedWrap::Init()

}
}