#include <napa-module.h>
#include <napa/object-transport-v8-helpers.h>
#include <plus-number-transporter.h>

namespace napa {
namespace demo {

using namespace v8;

// Since there is no host to store ObjectTransporter instance, make it globally for test.
std::unique_ptr<module::ObjectTransporter> _objectTransporter;

void CreatePlusNumberTransporter(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();
    HandleScope scope(isolate);

    CHECK_ARG(isolate,
        args.Length() == 0 || args.Length() == 1,
        "Only one or no argument is allowed.");

    if (args.Length() == 1) {
        CHECK_ARG(isolate,
            args[0]->IsNumber(),
            "The first argument must be a number.");
    }

    double value = args[0]->IsUndefined() ? 0.0 : args[0]->NumberValue();
    auto plusNumberTransporter = std::make_shared<PlusNumberTransporter>(value);

    auto transporter = plusNumberTransporter->Serialize();
    _objectTransporter.swap(transporter.second);

    auto result = module::object_transport::UintptrToV8Uint32Array(isolate, transporter.first);
    args.GetReturnValue().Set(result);
}

void Add(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();
    HandleScope scope(isolate);

    CHECK_ARG(isolate,
        args.Length() == 1 || args.Length() == 2,
        "Only one or two arguments are allowed.");

    CHECK_ARG(isolate,
        args[0]->IsArray(),
        "The first argument must be an array");

    auto transporter = module::object_transport::V8Uint32ArrayToUintptr(isolate, Local<Array>::Cast(args[0]));
    CHECK_ARG(isolate,
        transporter.second,
        "The first argument has a wrong pointer representation.");

    if (args.Length() == 2) {
        CHECK_ARG(isolate,
            args[1]->IsNumber(),
            "The second argument must be a number.");
    }
    double value = args[1]->IsUndefined() ? 0.0 : args[1]->NumberValue();

    auto object = module::ObjectTransporter::Deserialize<PlusNumberTransporter>(transporter.first);
    CHECK_ARG(isolate,
        object != nullptr,
        "Can't deserialize ObjectTransporter containing PlusNumberTransporter instance");

    auto result = object->Add(value);
    args.GetReturnValue().Set(Number::New(isolate, result));
}

void InitAll(Local<Object> exports) {
    NAPA_SET_METHOD(exports, "createPlusNumberTransporter", CreatePlusNumberTransporter);
    NAPA_SET_METHOD(exports, "add", Add);
}

NAPA_MODULE(addon, InitAll);

}  // namespace demo
}  // namespace napa