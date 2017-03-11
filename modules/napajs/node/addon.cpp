#include <napa-module.h>

#include "napa.h"
#include "container-wrap.h"
#include "logging-provider-wrap.h"

#include <algorithm>

void Initialize(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

#ifdef NAPA_MODULE_EXTENSION
    JS_ASSERT(isolate, false, "napa.initialize cannot be called from inside napa");
#endif

    if (args.Length() <= 0) {
        // No settings provided.
        napa::InitializeFromConsole(0, nullptr);
    } else {
        CHECK_ARG(
            isolate,
            args[0]->IsString() || args[0]->IsObject(),
            "first argument to initialize must be a string or an object");

        if (args[0]->IsString()) {
            // Settings provided as string.

            v8::String::Utf8Value settings(args[0]->ToString(context).ToLocalChecked());
            napa::Initialize(*settings);
        } else {
            // Settings provided as object.

            auto settingsObj = args[0]->ToObject(context).ToLocalChecked();

            auto settingsMap = napa::v8_helpers::V8ObjectToMap<std::string>(isolate, settingsObj);

            std::stringstream ss;
            for (const auto& kv : settingsMap) {
                ss << " --" << kv.first << " " << kv.second;
            }

            napa::Initialize(ss.str());
        }
    }
}

void Shutdown(const v8::FunctionCallbackInfo<v8::Value>&) {
#ifdef NAPA_MODULE_EXTENSION
    JS_ASSERT(v8::Isolate::GetCurrent(), false, "napa.shutdown cannot be called from inside napa");
#endif

    napa::Shutdown();
}

void CreateContainer(const v8::FunctionCallbackInfo<v8::Value>& args) {
#ifdef NAPA_MODULE_EXTENSION
    JS_ASSERT(v8::Isolate::GetCurrent(), false, "napa.createContainer cannot be called from inside napa");
#else
    napa::binding::ContainerWrap::NewInstance(args);
#endif
}

void GetLoggingProvider(const v8::FunctionCallbackInfo<v8::Value>& args) {
    napa::binding::LoggingProviderWrap::NewInstance(args);
}

void InitAll(v8::Local<v8::Object> exports) {
#ifndef NAPA_MODULE_EXTENSION
    napa::binding::ContainerWrap::Init(v8::Isolate::GetCurrent());
#endif

    napa::binding::LoggingProviderWrap::Init(v8::Isolate::GetCurrent());

    NAPA_SET_METHOD(exports, "initialize", Initialize);
    NAPA_SET_METHOD(exports, "shutdown", Shutdown);
    NAPA_SET_METHOD(exports, "createContainer", CreateContainer);
    NAPA_SET_METHOD(exports, "getLoggingProvider", GetLoggingProvider);
}

NAPA_MODULE(addon, InitAll)
