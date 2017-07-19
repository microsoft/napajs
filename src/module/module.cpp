// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <napa/module/module-internal.h>
#include <napa/v8-helpers.h>

#include <utils/debug.h>
#include <zone/worker-context.h>

using namespace napa;

/// <summary>
/// Map from module's class name to persistent constructor object.
/// To suppport multiple isolates, let each isolate has its own persistent constructor at thread local storage.
/// </summary>
typedef v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> PersistentConstructor;
struct ConstructorInfo {
    std::unordered_map<std::string, PersistentConstructor> constructorMap;
};

/// <summary> It sets the persistent constructor at the current V8 isolate. </summary>
/// <param name="name"> Unique constructor name. It's recommended to use the same name as module. </param>
/// <param name="constructor"> V8 persistent function to constructor V8 object. </param>
void napa::module::SetPersistentConstructor(const char* name,
                                     v8::Local<v8::Function> constructor) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto constructorInfo =
        static_cast<ConstructorInfo*>(zone::WorkerContext::Get(zone::WorkerContextItem::CONSTRUCTOR));
    if (constructorInfo == nullptr) {
        constructorInfo = new ConstructorInfo();
        zone::WorkerContext::Set(zone::WorkerContextItem::CONSTRUCTOR, constructorInfo);
    }

    constructorInfo->constructorMap.emplace(std::piecewise_construct,
                                            std::forward_as_tuple(name),
                                            std::forward_as_tuple(isolate, constructor));
}

/// <summary> It gets the given persistent constructor from the current V8 isolate. </summary>
/// <param name="name"> Unique constructor name given at SetPersistentConstructor() call. </param>
/// <returns> V8 local function object. </returns>
v8::Local<v8::Function> napa::module::GetPersistentConstructor(const char* name) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope scope(isolate);

    auto constructorInfo =
        static_cast<ConstructorInfo*>(zone::WorkerContext::Get(zone::WorkerContextItem::CONSTRUCTOR));
    if (constructorInfo == nullptr) {
        return scope.Escape(v8::Local<v8::Function>());
    }

    auto iter = constructorInfo->constructorMap.find(name);
    if (iter != constructorInfo->constructorMap.end()) {
        auto constructor = v8::Local<v8::Function>::New(isolate, iter->second);
        return scope.Escape(constructor);
    } else {
        return scope.Escape(v8::Local<v8::Function>());
    }
}
