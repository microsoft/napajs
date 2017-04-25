#pragma once
#include <napa-module.h>
#include <napa/memory/allocator.h>
#include <napa/module/shared-wrap.h>

namespace napa {
namespace module {
    /// <summary> Interface for AllocatorWrap. </summary>
    class AllocatorWrap: public SharedWrap {
    public:
        /// <summary> Get transport context. </summary>
        std::shared_ptr<napa::memory::Allocator> Get() {
            return SharedWrap::Get<napa::memory::Allocator>();
        }

        /// <summary> Create an allocator wrap with an allocator. </summary>
        static v8::Local<v8::Object> Create(std::shared_ptr<napa::memory::Allocator> allocator) {
            return SharedWrap::Create<napa::memory::Allocator, AllocatorWrap>(allocator);
        }
        
        /// <summary> It creates a persistent constructor for SharedWrap instance. </summary>
        static void Init() {
            auto isolate = v8::Isolate::GetCurrent();
            auto constructorTemplate = v8::FunctionTemplate::New(isolate, ConstructorCallback);
            constructorTemplate->SetClassName(v8_helpers::MakeV8String(isolate, exportName));
            constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);

            InitConstructorTemplate<AllocatorWrap>(constructorTemplate);
            auto constructor = constructorTemplate->GetFunction();
            InitConstructor("<AllocatorWrap>", constructor);
            NAPA_SET_PERSISTENT_CONSTRUCTOR(exportName, constructor);
        }

        /// <summary> It initialize constructor template of AllocatorWrap or sub-class to have common Allocator methods. </summary>
        /// <param name="constructorTemplate"> Constructor template of wrap class. </param>
        /// <remarks> Should call this method in sub-class Init. </remarks>
        template <typename WrapType>
        static void InitConstructorTemplate(v8::Local<v8::FunctionTemplate> constructorTemplate) {
            // Blessed with methods from napajs.transport.Transportable.
            SharedWrap::InitConstructorTemplate<WrapType>(constructorTemplate);

            NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "allocate", WrapType::AllocateCallback);
            NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "deallocate", WrapType::DeallocateCallback);
            NAPA_SET_ACCESSOR(constructorTemplate, "type", WrapType::GetTypeCallback, nullptr);
        }

        /// <summary> It initialize constructor of SharedWrap or sub-class. </summary>
        /// <remarks> Should call this method in sub-class Init. </remarks>
        /// <param name="cid"> Cid used for transporting the wrap. </param>
        /// <param name='constructor'> Constructor of wrap class. </param>
        static void InitConstructor(const char* cid, v8::Local<v8::Function> constructor) {
            SharedWrap::InitConstructor(cid, constructor);
        }

        /// <summary> Exported class name. </summary>
        static constexpr const char* exportName = "AllocatorWrap";

        /// <summary> Declare constructor in public, so we can export class constructor in JavaScript world. </summary>
        NAPA_DECLARE_PERSISTENT_CONSTRUCTOR

    protected:
       
        /// <summary> It creates SharedWrap from Javascript world. </summary>
        static void ConstructorCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
            auto isolate = v8::Isolate::GetCurrent();
            v8::HandleScope scope(isolate);

            CHECK_ARG(isolate, args.Length() == 0, "class \"AllocatorWrap\" doesn't accept any arguments in constructor.'");
            JS_ENSURE(isolate, args.IsConstructCall(), "class \"AllocatorWrap\" allows constructor call only.");

            // It's deleted when its Javascript object is garbage collected by V8's GC.
            auto wrap = new AllocatorWrap();
            wrap->Wrap(args.This());
            args.GetReturnValue().Set(args.This());
        }

        /// <summary> It implements Allocator.allocate(size: number): napajs.memory.Handle </summary>
        static void AllocateCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
            auto isolate = v8::Isolate::GetCurrent();
            v8::HandleScope scope(isolate);

            CHECK_ARG(isolate, args.Length() == 1, "1 argument of 'size' is required for \"allocate\".");
            CHECK_ARG(isolate, args[0]->IsUint32(), "Argument \"size\" must be a unsigned integer.");

            auto thisObject = NAPA_OBJECTWRAP::Unwrap<AllocatorWrap>(args.Holder());
            auto allocator = thisObject->Get();
            JS_ENSURE(isolate, allocator != nullptr, "AllocatorWrap is not attached with any C++ allocator.");

            auto handle = v8_helpers::PtrToV8Uint32Array(isolate, allocator->Allocate(args[0]->Uint32Value()));
            args.GetReturnValue().Set(handle);
        }

        /// <summary> It implements Allocator.allocate(size: number): napajs.memory.Handle </summary>
        static void DeallocateCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
            auto isolate = v8::Isolate::GetCurrent();
            v8::HandleScope scope(isolate);

            CHECK_ARG(isolate, args.Length() == 2, "2 arguments is required for \"deallocate\".");
            auto result = v8_helpers::V8ValueToUintptr(isolate, args[0]);
            JS_ENSURE(isolate, result.second, "Unable to cast \"handle\" to pointer. Please check if it's in valid handle format.");
            
            CHECK_ARG(isolate, args[1]->IsUint32(), "Argument \"sizeHint\" must be a 32-bit unsigned integer.");
            auto thisObject = NAPA_OBJECTWRAP::Unwrap<AllocatorWrap>(args.Holder());
            auto allocator = thisObject->Get();
            JS_ENSURE(isolate, allocator != nullptr, "AllocatorWrap is not attached with any C++ allocator.");
            
            allocator->Deallocate(reinterpret_cast<void*>(result.first), args[1]->Uint32Value());
        }

        /// <summary> It implments readonly Allocator.type: string </summary>
        static void GetTypeCallback(v8::Local<v8::String> propertyName, const v8::PropertyCallbackInfo<v8::Value>& args){
            auto isolate = v8::Isolate::GetCurrent();
            v8::HandleScope scope(isolate);
            auto thisObject = NAPA_OBJECTWRAP::Unwrap<AllocatorWrap>(args.Holder());
            auto allocator = thisObject->Get();
            args.GetReturnValue().Set(
                v8_helpers::MakeV8String(
                    isolate, 
                    allocator != nullptr ? allocator->GetType() : "<unloaded>"));
        }
    };

    /// <summary> It defines the persistent constructor for AllocatorWrap class. <summary>
    /// <remarks> Any module using ShardWrap must call this macro. </remarks>
    #define NAPA_DEFINE_PERSISTENT_ALLOCATOR_WRAP_CONSTRUCTOR() NAPA_DEFINE_PERSISTENT_CONSTRUCTOR(napa::module::AllocatorWrap)

    /// <summary> It creates and registers the persistent constructor for AllocatorWrap class. <summary>
    /// <remarks> Any module using AllocatorWrap must run this macro. </remarks>
    #define NAPA_ALLOCATOR_WRAP_INIT() napa::module::AllocatorWrap::Init()
}
}
    