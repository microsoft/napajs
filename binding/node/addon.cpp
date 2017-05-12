#include "napa-binding.h"

#include <napa.h>
#include <napa-module.h>

void Initialize(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    auto context = isolate->GetCurrentContext();

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
    napa::Shutdown();
}

void InitAll(v8::Local<v8::Object> exports) {
    napa::module::binding::Init(exports);

    // Only node addon can initialize/shutdown napa.
    NAPA_SET_METHOD(exports, "initialize", Initialize);
    NAPA_SET_METHOD(exports, "shutdown", Shutdown);
}

NAPA_MODULE(addon, InitAll)
