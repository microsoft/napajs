// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/module.h>
#include <napa/transport/transport-context.h>
#include <napa/transport/transport.h>

namespace napa {
namespace transport {

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
            napa::transport::Register(constructor);
        }

        /// <summary> It implements Transportable.cid() : string </summary>
        static void GetCidCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
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
            v8::HandleScope scope(isolate);
            auto context = isolate->GetCurrentContext();

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
            v8::HandleScope scope(isolate);
            auto context = isolate->GetCurrentContext();

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
} // namespace transport
} // namespace napa
