#include "console.h"

// This is not a module extension, so define this macro to use V8 common macros.
#define NAPA_MODULE_EXTENSION
#include <napa-module.h>

#include <iostream>
#include <sstream>

using namespace napa;
using namespace napa::module;

namespace {

    /// <summary> Log a message to console. </summary>
    /// <param name="args"> All arguments are converted to string and printed out to console. </param>
    void LogCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

}   // End of anonymous namespace.

void console::Init(v8::Local<v8::Context> context) {
    auto isolate = v8::Isolate::GetCurrent();

    auto console = v8::ObjectTemplate::New(isolate);

    NAPA_SET_METHOD(console, "log", LogCallback);

    (void)context->Global()->CreateDataProperty(isolate->GetCurrentContext(),
                                                v8_helpers::MakeV8String(isolate, "console"),
                                                console->NewInstance());
}

namespace {

    void LogCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        std::ostringstream oss;
        for (int i = 0; i < args.Length(); ++i) {
            v8::String::Utf8Value argument(args[i]->ToString());
            oss << *argument << " ";
        }

        std::string message = oss.str();
        if (!message.empty()) {
            message.pop_back();
        }

        std::cout << message << std::endl;

        args.GetReturnValue().Set(args.Holder());
    }

}   // End of anonymous namespace.