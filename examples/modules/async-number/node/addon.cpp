#include <napa-module.h>

#include <atomic>
#include <functional>

namespace napa {
namespace demo {

using namespace v8;

namespace {
    std::atomic<uint32_t> _now(0);
}

/// <summary>
/// It increases a number by a given parameter in separate thread.
/// When asynchronous work completes, it increases by one again at callback,
/// so we can verify that async-complete callback is called after a current context's run is done.
/// </summary>
void Increase(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();

    CHECK_ARG(isolate,
        args.Length() == 2 && args[0]->IsUint32() && args[1]->IsFunction(),
        "It requires unsigned integer and callback as arguments");

    auto value = args[0]->Uint32Value();

    NAPA_POST_ASYNC_WORK(
        Local<Function>::Cast(args[1]),
        [value]() {
            // This runs at the separate thread.
            _now += value;
        },
        [](auto jsCallback) {
            // This runs at the same thread as one Increase() is called.
            auto isolate = Isolate::GetCurrent();

            int32_t argc = 1;
            Local<Value> argv[] = { Integer::NewFromUnsigned(isolate, ++_now) };

            jsCallback->Call(isolate->GetCurrentContext()->Global(), argc, argv);
        });
}

/// <summary> It returns the current value of a number. </summary>
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

}  // namespace demo
}  // namespace napa
