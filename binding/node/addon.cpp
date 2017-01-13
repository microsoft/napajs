// Node uses deprecated V8 APIs
#pragma warning(push)
#pragma warning(disable: 4996)
#include <node.h>
#pragma warning(pop)

#include "napa/v8-helpers.h"
#include "napa-runtime.h"
#include "container-wrap.h"

#include <algorithm>

void Initialize(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    auto context = isolate->GetCurrentContext();

    if (args.Length() <= 0) {
        // No settings provided.
        napa::runtime::InitializeFromConsole(0, nullptr);
    } else {
        CHECK_ARG(
            isolate,
            args[0]->IsString() || args[0]->IsObject(),
            "first argument to initialize must be a string or an object");

        if (args[0]->IsString()) {
            // Settings provided as string.

            v8::String::Utf8Value settings(args[0]->ToString());
            napa::runtime::Initialize(*settings);
        } else {
            // Settings provided as object.

            auto settingsObj = args[0]->ToObject(context).ToLocalChecked();

            auto settingsMap = napa::v8_helpers::V8ObjectToMap<std::string>(isolate, settingsObj);

            std::stringstream ss;
            for (const auto& kv : settingsMap) {
                ss << " --" << kv.first << " " << kv.second;
            }

            napa::runtime::Initialize(ss.str());
        }
    }
}

void Shutdown(const v8::FunctionCallbackInfo<v8::Value>& args) {
    napa::runtime::Shutdown();
}

void CreateContainer(const v8::FunctionCallbackInfo<v8::Value>& args) {
    napa::binding::ContainerWrap::NewInstance(args);
}

void InitAll(v8::Local<v8::Object> exports) {
    napa::binding::ContainerWrap::Init(v8::Isolate::GetCurrent());

    NODE_SET_METHOD(exports, "initialize", Initialize);
    NODE_SET_METHOD(exports, "shutdown", Shutdown);
    NODE_SET_METHOD(exports, "createContainer", CreateContainer);
}

NODE_MODULE(napa_wrap, InitAll)
