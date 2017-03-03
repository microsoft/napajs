#include <napa-log.h>
#include <napa/shared-wrap.h>

using namespace napa;
using namespace napa::module;

NAPA_DEFINE_PERSISTENT_SHARED_WRAP_CONSTRUCTOR

struct SharedString {

    void Set(const char* value) {
        std::lock_guard<std::mutex> lock(_mutex);
        _value.assign(value);
    }

    std::string Get() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _value;
    }

    std::mutex _mutex;
    std::string _value;
};

// Create a dummy SharedWrap object for test.
void CreateTestableSharedString(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);

    auto objectWrap = SharedWrap::Create(std::make_shared<SharedString>());
    args.GetReturnValue().Set(objectWrap);
}

void SetTestableSharedString(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate,
        args.Length() == 2,
        "Two arguments are required.");

    CHECK_ARG(isolate,
        args[0]->IsObject(),
        "Shared object must be given as the first argument.");

    CHECK_ARG(isolate,
        args[1]->IsString(),
        "String must be given as the second argument.");

    auto objectWrap = NAPA_OBJECTWRAP::Unwrap<SharedWrap>(args[0]->ToObject());
    assert(objectWrap != nullptr);
    auto object = objectWrap->Get<SharedString>();

    v8::String::Utf8Value value(args[1]->ToString());
    object->Set(*value);

    args.GetReturnValue().SetUndefined();
}

void ReadTestableSharedString(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate,
        args.Length() == 1 && args[0]->IsObject(),
        "Shared object must be given as an argument.");

    auto objectWrap = NAPA_OBJECTWRAP::Unwrap<SharedWrap>(args[0]->ToObject());
    NAPA_ASSERT(objectWrap != nullptr, "Can't unwrap a shared object of SharedString type.");
    auto object = objectWrap->Get<SharedString>();
    std::string value = object->Get();

    args.GetReturnValue().Set(v8_helpers::MakeV8String(isolate, value.c_str()));
}

void InitAll(v8::Local<v8::Object> exports) {
    NAPA_SHARED_WRAP_INIT();

    NAPA_SET_METHOD(exports, "createTestableSharedString", CreateTestableSharedString);
    NAPA_SET_METHOD(exports, "setTestableSharedString", SetTestableSharedString);
    NAPA_SET_METHOD(exports, "readTestableSharedString", ReadTestableSharedString);
}

NAPA_MODULE(dummy, InitAll);