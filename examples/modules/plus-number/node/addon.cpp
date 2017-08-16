// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "addon.h"

using namespace napa::demo;
using namespace v8;

const char* PlusNumberWrap::_exportName = "PlusNumberWrap";

// Define persistent constructor.
NAPA_DEFINE_PERSISTENT_CONSTRUCTOR(PlusNumberWrap)

PlusNumberWrap::PlusNumberWrap(double value) : _plusNumber(value) {}

void PlusNumberWrap::Init() {
    auto isolate = Isolate::GetCurrent();

    // Prepare constructor template.
    auto functionTemplate = FunctionTemplate::New(isolate, NewCallback);
    functionTemplate->SetClassName(String::NewFromUtf8(isolate, _exportName));
    functionTemplate->InstanceTemplate()->SetInternalFieldCount(1);

    // Set prototype method.
    NAPA_SET_PROTOTYPE_METHOD(functionTemplate, "add", Add);

    // Set persistent constructor into V8.
    NAPA_SET_PERSISTENT_CONSTRUCTOR(_exportName, functionTemplate->GetFunction());
}

void PlusNumberWrap::NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    HandleScope scope(isolate);

    const int argc = 1;
    Local<Value> argv[argc] = { args[0] };

    auto constructor = NAPA_GET_PERSISTENT_CONSTRUCTOR(_exportName, PlusNumberWrap);
    auto context = isolate->GetCurrentContext();
    auto instance = constructor->NewInstance(context, argc, argv).ToLocalChecked();

    args.GetReturnValue().Set(instance);
}

void PlusNumberWrap::NewCallback(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();
    HandleScope scope(isolate);

    CHECK_ARG(isolate, args.IsConstructCall(), "PlusNumberWrap instance must be created by the factory.");

    CHECK_ARG(isolate, args.Length() == 0 || args.Length() == 1, "Only one or no argument is allowed.");

    if (args.Length() == 1) {
        CHECK_ARG(isolate, args[0]->IsNumber(), "The first argument must be a number.");
    }

    double value = args[0]->IsUndefined() ? 0.0 : args[0]->NumberValue();

    auto wrap = new PlusNumberWrap(value);
    wrap->Wrap(args.This());

    args.GetReturnValue().Set(args.This());
}

void PlusNumberWrap::Add(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();
    HandleScope scope(isolate);

    CHECK_ARG(isolate, args.Length() == 1 && args[0]->IsNumber(), "Number must be given as argument.");

    auto wrap = NAPA_OBJECTWRAP::Unwrap<PlusNumberWrap>(args.Holder());
    auto value = wrap->_plusNumber.Add(args[0]->NumberValue());

    args.GetReturnValue().Set(Number::New(isolate, value));
}

void CreatePlusNumber(const FunctionCallbackInfo<Value>& args) {
    PlusNumberWrap::NewInstance(args);
}

void InitAll(Local<Object> exports) {
    PlusNumberWrap::Init();

    NAPA_SET_METHOD(exports, "createPlusNumber", CreatePlusNumber);
}

NAPA_MODULE(addon, InitAll);
