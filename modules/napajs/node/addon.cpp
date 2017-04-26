#include <napa.h>
#include <napa-memory.h>
#include <napa-module.h>
#include <napa/module/shared-wrap.h>

#include "zone-wrap.h"
#include "logging-provider-wrap.h"
#include "transport-context-wrap-impl.h"
#include "simple-allocator-debugger-wrap.h"

#include <algorithm>

using namespace napa::binding;

NAPA_DEFINE_PERSISTENT_SHARED_WRAP_CONSTRUCTOR();
NAPA_DEFINE_PERSISTENT_ALLOCATOR_WRAP_CONSTRUCTOR();

void Initialize(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

#ifdef NAPA_MODULE_EXTENSION
    JS_ASSERT(isolate, false, "napa.initialize cannot be called from inside napa");
#endif

    if (args.Length() <= 0 || args[0]->IsUndefined()) {
        // No settings provided.
        napa::InitializeFromConsole(0, nullptr);
    } else {
        CHECK_ARG(isolate, args[0]->IsObject(), "first argument to initialize must be an object");

        auto settingsObj = args[0]->ToObject(context).ToLocalChecked();

        auto settingsMap = napa::v8_helpers::V8ObjectToMap<std::string>(isolate, settingsObj);

        std::stringstream ss;
        for (const auto& kv : settingsMap) {
            ss << " --" << kv.first << " " << kv.second;
        }

        napa::Initialize(ss.str());
    }
}

void Shutdown(const v8::FunctionCallbackInfo<v8::Value>&) {
#ifdef NAPA_MODULE_EXTENSION
    JS_ASSERT(v8::Isolate::GetCurrent(), false, "napa.shutdown cannot be called from inside napa");
#endif

    napa::Shutdown();
}

void CreateZone(const v8::FunctionCallbackInfo<v8::Value>& args) {
#ifdef NAPA_MODULE_EXTENSION
    JS_ASSERT(v8::Isolate::GetCurrent(), false, "napa.createZone cannot be called from inside napa");
#else
    ZoneWrap::NewInstance(ZoneWrap::ConstructorType::CREATE, args);
#endif
}

void GetZone(const v8::FunctionCallbackInfo<v8::Value>& args) {
#ifdef NAPA_MODULE_EXTENSION
    JS_ASSERT(v8::Isolate::GetCurrent(), false, "napa.getZone cannot be called from inside napa");
#else
    ZoneWrap::NewInstance(ZoneWrap::ConstructorType::GET, args);
#endif
}

void GetCurrentZone(const v8::FunctionCallbackInfo<v8::Value>& args) {
#ifdef NAPA_MODULE_EXTENSION
    JS_ASSERT(v8::Isolate::GetCurrent(), false, "napa.getCurrentZone cannot be called from inside napa");
#else
    ZoneWrap::NewInstance(ZoneWrap::ConstructorType::CURRENT, args);
#endif
}

void GetLoggingProvider(const v8::FunctionCallbackInfo<v8::Value>& args) {
    LoggingProviderWrap::NewInstance(args);
}

void GetResponseCodeString(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    CHECK_ARG(isolate, args[0]->IsUint32(), "response code must be a valid uint32");

    auto responseCode = args[0]->Uint32Value();

    auto responseCodeString = napa_response_code_to_string(static_cast<napa_response_code>(responseCode));
    args.GetReturnValue().Set(napa::v8_helpers::MakeV8String(isolate, responseCodeString));
}

void GetCrtAllocator(const v8::FunctionCallbackInfo<v8::Value>& args) {
    args.GetReturnValue().Set(
        napa::module::AllocatorWrap::Create(
            NAPA_MAKE_SHARED<napa::memory::CrtAllocator>()));
}

void GetDefaultAllocator(const v8::FunctionCallbackInfo<v8::Value>& args) {
    args.GetReturnValue().Set(
        napa::module::AllocatorWrap::Create(
            NAPA_MAKE_SHARED<napa::memory::DefaultAllocator>()));
}

void InitAll(v8::Local<v8::Object> exports) {
    NAPA_SHARED_WRAP_INIT();
    NAPA_ALLOCATOR_WRAP_INIT();

#ifndef NAPA_MODULE_EXTENSION
    napa::binding::ZoneWrap::Init(v8::Isolate::GetCurrent());
#endif

    napa::binding::LoggingProviderWrap::Init(v8::Isolate::GetCurrent());
    napa::binding::TransportContextWrapImpl::Init();
    napa::binding::SimpleAllocatorDebuggerWrap::Init();

    NAPA_EXPORT_OBJECTWRAP(exports, "SharedWrap", napa::module::SharedWrap);
    NAPA_EXPORT_OBJECTWRAP(exports, "AllocatorWrap", napa::module::AllocatorWrap);
    NAPA_EXPORT_OBJECTWRAP(exports, "TransportContextWrap", napa::binding::TransportContextWrapImpl);
    NAPA_EXPORT_OBJECTWRAP(exports, "SimpleAllocatorDebuggerWrap", napa::binding::SimpleAllocatorDebuggerWrap);

    NAPA_SET_METHOD(exports, "initialize", Initialize);
    NAPA_SET_METHOD(exports, "shutdown", Shutdown);

    NAPA_SET_METHOD(exports, "createZone", CreateZone);
    NAPA_SET_METHOD(exports, "getZone", GetZone);
    NAPA_SET_METHOD(exports, "getCurrentZone", GetCurrentZone);

    NAPA_SET_METHOD(exports, "getLoggingProvider", GetLoggingProvider);
    NAPA_SET_METHOD(exports, "getResponseCodeString", GetResponseCodeString);

    NAPA_SET_METHOD(exports, "getCrtAllocator", GetCrtAllocator);
    NAPA_SET_METHOD(exports, "getDefaultAllocator", GetDefaultAllocator);
}

NAPA_MODULE(addon, InitAll)
