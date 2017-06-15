#include "tty-wrap.h"

#include <napa-module.h>
#include <napa/module/platform.h>

using namespace napa;
using namespace napa::module;

namespace {

    /// <summary> Callback to isTTY(). </summary>
    void IsTTYCallback(const v8::FunctionCallbackInfo<v8::Value>&);

    /// <summary> Callback to guessHandleType(). </summary>
    void GuessHandleTypeCallback(const v8::FunctionCallbackInfo<v8::Value>&);

}   // End of anonymous namespace.

void tty_wrap::Init(v8::Local<v8::Object> exports) {
    NAPA_SET_METHOD(exports, "isTTY", IsTTYCallback);
    NAPA_SET_METHOD(exports, "guessHandleType", GuessHandleTypeCallback);
}

namespace {

    void IsTTYCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        CHECK_ARG(isolate,
            args.Length() == 1 && args[0]->IsInt32(),
            "tty_wrap.isTTY requires a file descriptor");

        int32_t fd = args[0]->Int32Value();
        CHECK_ARG(isolate, fd >= 0, "Invalid file descriptor");

        args.GetReturnValue().Set(platform::Isatty(fd) > 0);
    }

    void GuessHandleTypeCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        CHECK_ARG(isolate,
            args.Length() == 1 && args[0]->IsInt32(),
            "tty_wrap.getHandleType requires a integer parameter");

        int32_t fd = args[0]->Int32Value();
        CHECK_ARG(isolate, fd >= 0, "Invalid file descriptor");

        // Napa doesn't have any information about handle type, so return as unknown handle.
        args.GetReturnValue().Set(v8_helpers::MakeV8String(isolate, "UNKNOWN"));
    }

}   // End of anonymous namespace.