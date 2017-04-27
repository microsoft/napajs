#pragma once

#include <napa-module.h>
#include <napa/transport/transport-context.h>
#include <napa/module/common.h>

namespace napa {
namespace module {

    namespace transport {
        void Register(v8::Local<v8::Function> constructor);
    }

    /// <summary> Helper for extending TransportableObject </summary>
    /// <remarks> Reference: napajs/lib/transport/transportable.ts#TransportableObject </remarks>
    struct TransportableObject {

        /// <summary> It initialize a wrap object constructor template with TransportableObject methods. </summary>
        /// <param name='constructorTemplate'> Constructor function template of the wrap. </param>
        static void InitConstructorTemplate(v8::Local<v8::FunctionTemplate> constructorTemplate) {
            NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "marshall", TransportableObject::MarshallCallback);
            NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "unmarshall", TransportableObject::UnmarshallCallback);
            NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "cid", TransportableObject::GetCidCallback);
        }

        /// <summary> It initialize a wrap constructor with static property _cid. </summary>
        /// <param name="cid"> Cid used for transporting the wrap. </param>
        /// <param name='constructor'> Constructor function of the wrap. </param>
        static void InitConstructor(const char* cid, v8::Local<v8::Function> constructor) {
            auto isolate = v8::Isolate::GetCurrent();
            constructor->Set(v8_helpers::MakeV8String(isolate, "_cid"), v8_helpers::MakeV8String(isolate, cid));

            //transport::Register(constructor);
        }

        /// <summary> It implements Transportable.cid() : string </summary>
        static void GetCidCallback(const v8::FunctionCallbackInfo<v8::Value>& args){
            auto isolate = v8::Isolate::GetCurrent();
            v8::HandleScope scope(isolate);
            auto prototype = v8::Local<v8::Object>::Cast(args.Holder()->GetPrototype());
            auto constructor = v8::Local<v8::Function>::Cast(prototype->Get(
                v8_helpers::MakeV8String(isolate, "constructor")));
            args.GetReturnValue().Set(constructor->Get(v8_helpers::MakeV8String(isolate, "_cid")));
        }

        /// <summary> It implements Transportable.marshall(context: TransportContext): object </summary>
        static void MarshallCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
            auto isolate = v8::Isolate::GetCurrent();
            auto context = isolate->GetCurrentContext();
            v8::HandleScope scope(isolate);

            CHECK_ARG(isolate, args.Length() == 1, "1 argument is required for calling 'marshall'.");
            CHECK_ARG(isolate, args[0]->IsObject(), "The 1st argument of 'marshall' shall be object of TransportContext.");

            // Get cid from property '_cid' of constructor and class name.
            auto holder = args.Holder();
            auto proto = v8::Local<v8::Object>::Cast(holder->GetPrototype());
            JS_ENSURE(isolate, !proto.IsEmpty(), "Prototype is not Object type");

            auto constructor = v8::Local<v8::Function>::Cast(
                proto->Get(v8_helpers::MakeV8String(isolate, "constructor")));

            auto cid = constructor->Get(v8_helpers::MakeV8String(isolate, "_cid"));

            // Save property "_cid".
            auto payload = v8::Object::New(isolate);
            payload->CreateDataProperty(
                context, 
                v8_helpers::MakeV8String(isolate, "_cid"), 
                cid);
            
            // Delegate to sub-class to save its members.
            auto saveMethod = v8::Local<v8::Function>::Cast(
                holder->Get(v8_helpers::MakeV8String(isolate, "save")));
            JS_ENSURE(isolate, 
                !saveMethod.IsEmpty(), 
                "\"save\" method doesn't exist.");
            
            constexpr int argc = 2;
            v8::Local<v8::Value> argv[argc] = {payload, args[0]};
            saveMethod->Call(
                context, 
                holder,
                argc,
                argv);

            args.GetReturnValue().Set(payload);
        }

        /// <summary> It implements Transportable.unmarshall(payload: object, context: TransportContext): void </summary>
        static void UnmarshallCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
            auto isolate = v8::Isolate::GetCurrent();
            auto context = isolate->GetCurrentContext();
            v8::HandleScope scope(isolate);

            CHECK_ARG(isolate, args.Length() == 2, "Two arguments are required for calling 'unmarshall'. ");

            auto holder = args.Holder();

            // Delegate to sub-class to load its members.
            auto loadMethod = v8::Local<v8::Function>::Cast(holder->Get(v8_helpers::MakeV8String(isolate, "load")));
            JS_ENSURE(isolate, !loadMethod.IsEmpty(), "\"load\" method doesn't exist.");
           
            constexpr int argc = 2;
            v8::Local<v8::Value> argv[argc] = {args[0], args[1]};
            loadMethod->Call(
                context, 
                holder,
                argc,
                argv);
        }
    };

    /// <summary> Helper for implementing NonTransportable. </summary>
    /// <remarks> Reference: napajs/lib/transport/non-transportable.ts#@nontransportable </remarks>
    struct NonTransportable {
        /// <summary> It makes a wrap object extends NonTransportable.</summary>
        /// <param name='constructorTemplate'> Constructor function template of the wrap. </param>
        static void InitConstructor(v8::Local<v8::Function> constructor) {
            auto isolate = v8::Isolate::GetCurrent();
            v8::HandleScope scope(isolate);
            
            // Add static property '_cid' to class.
            constructor->Set(
                v8_helpers::MakeV8String(isolate, "_cid"), 
                v8_helpers::MakeV8String(isolate, "<non-transportable>"));
        }
    };

    /// <summary> Interface for TransportContextWrap. </summary>
    class TransportContextWrap: public NAPA_OBJECTWRAP {
    public:
        /// <summary> Get transport context. </summary>
        virtual napa::transport::TransportContext* Get() = 0;

        virtual ~TransportContextWrap() = default;
    };

    namespace transport {
        /// <summary> Marshall an object with transport context. C++ modules can use this helper function to marshall its members. </summary>
        /// <param name="object"> Object to marshall, it can be built-in JavaScript types or object implements napajs.transport.Transportable. </param>
        /// <param name="transportContextWrap"> TransportContextWrap to save shareable states if any. </param>
        /// <returns> Payload in V8 string of marshalled object. </summary>
        inline v8::MaybeLocal<v8::String> Marshall(v8::Local<v8::Value> object, v8::Local<v8::Object> transportContextWrap) {
            constexpr int argc = 2;
            v8::Local<v8::Value> argv[argc] = { object, transportContextWrap };
            v8_helpers::MaybeCast<v8::String>(v8_helpers::Call("napajs/lib/transport", "marshall", argc, argv));
        }

        /// <summary> Unmarshall a payload with transport context. C++ modules can use this helper function to unmarshall its members. </summary>
        /// <param name="payload"> Payload to unmarshall, it is plain JS object generated by transport.marshallSingle. </param>
        /// <param name="transportContextWrap"> TransportContextWrap to load shareable states if any. </param>
        /// <returns> Unmarshalled V8 value from payload. </summary>
        inline v8::MaybeLocal<v8::Value> Unmarshall(v8::Local<v8::Value> payload, v8::Local<v8::Object> transportContextWrap) {
            constexpr int argc = 2;
            v8::Local<v8::Value> argv[argc] = { payload, transportContextWrap };
            return v8_helpers::Call("napajs/lib/transport", "unmarshall", argc, argv);
        }

        /// <summary> Register a Transportable object wrap with transport. </summary>
        /// <param name="constructor"> Constructor of wrap type. </param>
        inline void Register(v8::Local<v8::Function> constructor) {
            constexpr int argc = 1;
            v8::Local<v8::Value> argv[argc] = { constructor };
            (void)v8_helpers::Call("napajs/lib/transport", "register", argc, argv);
        }
    }
}
}