#include <napa-module.h>

#include <functional>

using namespace napa;
using namespace napa::module;

using namespace v8;

namespace {
    uint32_t _now = 0;
}

void Increase(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();

    CHECK_ARG(isolate,
        args.Length() == 2 && args[0]->IsUint32() && args[1]->IsFunction(),
        "It requires unsigned integer and callback as arguments");

    auto value = args[0]->Uint32Value();

    NAPA_POST_ASYNC_WORK(
        Local<Function>::Cast(args[1]),
        [value]() {
            _now += value;
        },
        [](auto jsCallback) {
            auto isolate = Isolate::GetCurrent();

            int32_t argc = 1;
            Local<Value> argv[] = { Integer::NewFromUnsigned(isolate, ++_now) };

            jsCallback->Call(isolate->GetCurrentContext()->Global(), argc, argv);
        });

    args.GetReturnValue().SetUndefined();
}

void Now(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();
    HandleScope scope(isolate);

    args.GetReturnValue().Set(Integer::NewFromUnsigned(isolate, _now));
}

void Init(Local<Object> exports) {
    NAPA_SET_METHOD(exports, "increase", Increase);
    NAPA_SET_METHOD(exports, "now", Now);
}

NAPA_MODULE(addon, Init)