// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/module.h>

#include <store/store.h>

namespace napa {
namespace module {
    
    /// <summary> It wraps napa::store::Store. </summary>
    /// <remarks> Reference: napajs/lib/store.ts#Store </remarks>
    class StoreWrap: public node::ObjectWrap {
    public:
        /// <summary> Init this wrap. </summary>
        static void Init();

        /// <summary> It creates an instance of StoreWrap with a napa::store::Store pointer. </summary>
        static v8::Local<v8::Object> NewInstance(std::shared_ptr<napa::store::Store> store);

        /// <summary> Get napa::store::Store from wrap. </summary>
        napa::store::Store& Get();

    private:
        /// <summary> Default constructor. </summary>
        StoreWrap() = default;

        /// <summary> No copy allowed. </summary>
        StoreWrap(const StoreWrap&) = delete;
        StoreWrap& operator=(const StoreWrap&) = delete;

        /// <summary> It implements Store.set(key: string, value: any): void </summary>
        static void SetCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> It implements Store.get(key: string): any </summary>
        static void GetCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> It implements Store.has(key: string): boolean </summary>
        static void HasCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> It implements Store.delete(key: string): void </summary>
        static void DeleteCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> It implements Store.id </summary>
        static void GetIdCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& args);

        /// <summary> It implements Store.size </summary>
        static void GetSizeCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& args);
        
        /// <summary> Friend default constructor callback. </summary>
        template <typename T>
        friend void napa::module::DefaultConstructorCallback(const v8::FunctionCallbackInfo<v8::Value>&);

        template <typename T>
        friend v8::MaybeLocal<v8::Object> napa::module::NewInstance(int argc, v8::Local<v8::Value> argv[]);

        /// <summary> Exported class name. </summary>
        static constexpr const char* exportName = "StoreWrap";

        /// <summary> Hid constructor from public access. </summary>
        static v8::Persistent<v8::Function> _constructor;

        /// <summary> Store. </summary>
        std::shared_ptr<napa::store::Store> _store;
    };
}
}
