#include <napa-module.h>

using namespace napa;
using namespace napa::module;

class SampleNode : public NAPA_OBJECTWRAP {
public:

    static void Init() {
        auto isolate = v8::Isolate::GetCurrent();

        auto functionTemplate = v8::FunctionTemplate::New(isolate, NewCallback);
        functionTemplate->SetClassName(v8_helpers::MakeV8String(isolate, _exportName));
        functionTemplate->InstanceTemplate()->SetInternalFieldCount(1);

        NAPA_SET_PROTOTYPE_METHOD(functionTemplate, "getValue", GetValueCallback);
        NAPA_SET_ACCESSOR(functionTemplate, "score", GetScoreCallback, SetScoreCallback);

        NAPA_SET_PERSISTENT_CONSTRUCTOR(_exportName, functionTemplate->GetFunction());
    }

    static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        const int argc = 1;
        v8::Local<v8::Value> argv[argc] = { args[0] };

        auto constructor = NAPA_GET_PERSISTENT_CONSTRUCTOR(_exportName, SampleNode);
        auto context = isolate->GetCurrentContext();
        auto instance = constructor->NewInstance(context, argc, argv).ToLocalChecked();

        args.GetReturnValue().Set(instance);
    }

    static v8::Local<v8::Object> NewInstance(uint32_t score) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope scope(isolate);

        const int argc = 1;
        v8::Local<v8::Value> argv[argc] = { v8::Number::New(isolate, score) };

        auto constructor = NAPA_GET_PERSISTENT_CONSTRUCTOR(_exportName, SampleNode);
        return scope.Escape(constructor->NewInstance(argc, argv));
    }

private:

    static constexpr const char* _exportName = "SampleNode";

    explicit SampleNode(uint32_t score = 0) : _score(score) {}

    static void NewCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        JS_ASSERT(isolate, args.IsConstructCall(), "Only constructor call is allowed");

        uint32_t score = args[0]->IsUndefined() ? 0 : args[0]->Uint32Value();

        auto sampleNode = new SampleNode(score);
        sampleNode->Wrap(args.Holder());
        args.GetReturnValue().Set(args.Holder());
    }

    static void GetScoreCallback(v8::Local<v8::String>,
                                 const v8::PropertyCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        auto sampleNode = NAPA_OBJECTWRAP::Unwrap<SampleNode>(args.Holder());
        args.GetReturnValue().Set(v8::Number::New(isolate, sampleNode->_score));
    }

    static void SetScoreCallback(v8::Local<v8::String>,
                                 v8::Local<v8::Value> value,
                                 const v8::PropertyCallbackInfo<void>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        CHECK_ARG(isolate,
                  value->IsUint32(),
                  "Value must be a number.");

        auto sampleNode = NAPA_OBJECTWRAP::Unwrap<SampleNode>(args.Holder());
        sampleNode->_score = value->Uint32Value();

        args.GetReturnValue().SetUndefined();
    }

    static void GetValueCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        CHECK_ARG(isolate,
            args.Length() > 0 && !args[0]->IsUndefined(),
            "The first argument must be a string.");

        auto that = args.Holder();
        auto target = that->Get(args[0]->ToString())->ToObject();

        args.GetReturnValue().Set(target);
    }

    uint32_t _score;
};

class SampleContainer : public NAPA_OBJECTWRAP {
public:

    static void Init() {
        auto isolate = v8::Isolate::GetCurrent();

        auto functionTemplate = v8::FunctionTemplate::New(isolate, NewCallback);
        functionTemplate->SetClassName(v8_helpers::MakeV8String(isolate, _exportName));
        functionTemplate->InstanceTemplate()->SetInternalFieldCount(1);

        auto sampleNode = SampleNode::NewInstance(0);
        functionTemplate->InstanceTemplate()->Set(v8_helpers::MakeV8String(isolate, "node"), sampleNode);

        NAPA_SET_PROTOTYPE_METHOD(functionTemplate, "setNode", SetNodeCallback);
        NAPA_SET_PROTOTYPE_METHOD(functionTemplate, "getNode", GetNodeCallback);

        NAPA_SET_PERSISTENT_CONSTRUCTOR(_exportName, functionTemplate->GetFunction());
    }

    static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        auto constructor = NAPA_GET_PERSISTENT_CONSTRUCTOR(_exportName, SampleContainer);
        JS_ASSERT(isolate, !constructor.IsEmpty(), "No registered constructor.");

        auto context = isolate->GetCurrentContext();
        auto instance = constructor->NewInstance(context).ToLocalChecked();

        args.GetReturnValue().Set(instance);
    }

private:

    static constexpr const char* _exportName = "SampleContainer";

    static void NewCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        JS_ASSERT(isolate, args.IsConstructCall(), "Only constructor call is allowed");

        auto container = new SampleContainer();
        container->Wrap(args.Holder());
        args.GetReturnValue().Set(args.Holder());
    }

    static void SetNodeCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        CHECK_ARG(isolate,
            args.Length() > 0 && !args[0]->IsUndefined(),
            "The first argument must be a SampleNode instance.");

        auto that = args.Holder();
        that->CreateDataProperty(isolate->GetCurrentContext(),
                                 v8_helpers::MakeV8String(isolate, "node"),
                                 args[0]->ToObject());

        args.GetReturnValue().SetUndefined();
    }

    static void GetNodeCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        auto that = args.Holder();
        auto node = that->Get(v8_helpers::MakeV8String(isolate, "node"))->ToObject();

        args.GetReturnValue().Set(node);
    }
};

void CreateSampleNode(const v8::FunctionCallbackInfo<v8::Value>& args) {
    SampleNode::NewInstance(args);
}

void CreateSampleContainer(const v8::FunctionCallbackInfo<v8::Value>& args) {
    SampleContainer::NewInstance(args);
}

void Print(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate,
        args.Length() > 0 && !args[0]->IsUndefined(),
        "The first argument must be a string.");

    v8::String::Utf8Value value(args[0]);
    args.GetReturnValue().Set(v8_helpers::MakeV8String(isolate, *value));
}

void Init(v8::Local<v8::Object> exports) {
    SampleNode::Init();
    SampleContainer::Init();

    NAPA_SET_METHOD(exports, "createSampleNode", CreateSampleNode);
    NAPA_SET_METHOD(exports, "createSampleContainer", CreateSampleContainer);
    NAPA_SET_METHOD(exports, "print", Print);
}

NAPA_MODULE(addon, Init);
