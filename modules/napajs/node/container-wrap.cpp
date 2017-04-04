#include "container-wrap.h"

#include "node-async-handler.h"

#include <napa.h>
#include <napa/v8-helpers.h>

#include <sstream>
#include <vector>

using namespace napa::binding;
using namespace napa::v8_helpers;

NAPA_DEFINE_PERSISTENT_CONSTRUCTOR(ContainerWrap);

// Forward declaration.
static std::vector<v8::Local<v8::Value>> CreateResponseValues(
    v8::Isolate* isolate,
    const napa::Response& response);


ContainerWrap::ContainerWrap(std::unique_ptr<napa::Container> container) : 
    _container(std::move(container)) {}

void ContainerWrap::Init(v8::Isolate* isolate) {
    // Prepare constructor template.
    v8::Local<v8::FunctionTemplate> functionTemplate = v8::FunctionTemplate::New(isolate, NewCallback);
    functionTemplate->SetClassName(MakeV8String(isolate, _exportName));
    functionTemplate->InstanceTemplate()->SetInternalFieldCount(1);

    // Prototypes.
    NAPA_SET_PROTOTYPE_METHOD(functionTemplate, "load", Load);
    NAPA_SET_PROTOTYPE_METHOD(functionTemplate, "loadSync", LoadSync);
    NAPA_SET_PROTOTYPE_METHOD(functionTemplate, "loadFile", LoadFile);
    NODE_SET_PROTOTYPE_METHOD(functionTemplate, "loadFileSync", LoadFileSync);
    NODE_SET_PROTOTYPE_METHOD(functionTemplate, "run", Run);
    NODE_SET_PROTOTYPE_METHOD(functionTemplate, "runSync", RunSync);
    NODE_SET_PROTOTYPE_METHOD(functionTemplate, "runAll", RunAll);
    NODE_SET_PROTOTYPE_METHOD(functionTemplate, "runAllSync", RunAllSync);

    // Set persistent constructor into V8.
    NAPA_SET_PERSISTENT_CONSTRUCTOR(_exportName, functionTemplate->GetFunction());
}

void ContainerWrap::NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    int argc = args.Length();
    std::vector<v8::Local<v8::Value>> argv;
    argv.reserve(argc);
    for (int i = 0; i < argc; ++i) {
        argv.emplace_back(args[i]);
    }

    auto constructor = NAPA_GET_PERSISTENT_CONSTRUCTOR(_exportName);
    auto context = isolate->GetCurrentContext();
    auto instance = constructor->NewInstance(context, argc, argv.data()).ToLocalChecked();

    args.GetReturnValue().Set(instance);
}

void ContainerWrap::NewCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    JS_ASSERT(isolate, args.IsConstructCall(), "Only constructor call is allowed");

    std::stringstream ss;
    if (args.Length() > 0) {
        CHECK_ARG(isolate, args[0]->IsObject(), "first argument to createContainer must be an object");

        v8::Local<v8::Object> settingsObj = args[0]->ToObject(context).ToLocalChecked();

        auto settingsMap = napa::v8_helpers::V8ObjectToMap<std::string>(isolate, settingsObj);

        for (const auto& kv : settingsMap) {
            ss << " --" << kv.first << " " << kv.second;
        }
    }

    auto obj = new ContainerWrap(std::make_unique<napa::Container>(ss.str()));

    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
}

void ContainerWrap::Load(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();

    CHECK_ARG(isolate, args[0]->IsString(), "first parameter to container.load must be the javascript source");
    CHECK_ARG(isolate, args[1]->IsFunction(), "second parameter to container.load must be the callback");
    
    v8::String::Utf8Value source(args[0]->ToString());
    auto callback = v8::Local<v8::Function>::Cast(args[1]);

    auto handler = NodeAsyncHandler<NapaResponseCode>::New(
        isolate,
        callback,
        [isolate](const auto& responseCode) {
            std::vector<v8::Local<v8::Value>> res;
            res.push_back(v8::Uint32::NewFromUnsigned(isolate, responseCode));
            return res;
        }
    );

    auto wrap = ObjectWrap::Unwrap<ContainerWrap>(args.Holder());
    wrap->_container->Load(*source, [handler](NapaResponseCode responseCode) {
        handler->DispatchCallback(responseCode);
    });
}

void ContainerWrap::LoadSync(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();

    CHECK_ARG(isolate, args[0]->IsString(), "first parameter to container.loadSync must be the javascript source");

    v8::String::Utf8Value source(args[0]->ToString());

    auto wrap = ObjectWrap::Unwrap<ContainerWrap>(args.Holder());
    auto responseCode = wrap->_container->LoadSync(*source);

    args.GetReturnValue().Set(v8::Uint32::NewFromUnsigned(isolate, responseCode));
}

void ContainerWrap::LoadFile(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();

    CHECK_ARG(isolate, args[0]->IsString(), "first parameter to container.loadFile must be the javascript file");
    CHECK_ARG(isolate, args[1]->IsFunction(), "second parameter to container.loadFile must be the callback");

    v8::String::Utf8Value file(args[0]->ToString());
    auto callback = v8::Local<v8::Function>::Cast(args[1]);

    auto handler = NodeAsyncHandler<NapaResponseCode>::New(
        isolate,
        callback,
        [isolate](const auto& responseCode) {
            std::vector<v8::Local<v8::Value>> res;
            res.push_back(v8::Uint32::NewFromUnsigned(isolate, responseCode));
            return res;
        }
    );

    auto wrap = ObjectWrap::Unwrap<ContainerWrap>(args.Holder());
    wrap->_container->LoadFile(*file, [handler](NapaResponseCode responseCode) {
        handler->DispatchCallback(responseCode);
    });
}

void ContainerWrap::LoadFileSync(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();

    CHECK_ARG(isolate, args[0]->IsString(), "first parameter to container.loadFileSync must be the javascript file");

    v8::String::Utf8Value file(args[0]->ToString());

    auto wrap = ObjectWrap::Unwrap<ContainerWrap>(args.Holder());
    auto responseCode = wrap->_container->LoadFileSync(*file);
    
    args.GetReturnValue().Set(v8::Uint32::NewFromUnsigned(isolate, responseCode));
}

void ContainerWrap::Run(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    CHECK_ARG(isolate, args[0]->IsString(), "first parameter to container.run must be the function name");
    CHECK_ARG(isolate, args[1]->IsArray(), "second parameter to container.run must be the arguments array");
    CHECK_ARG(isolate, args[2]->IsFunction(), "third parameter to container.run must be the callback");

    if (args.Length() > 3) {
        CHECK_ARG(isolate, args[3]->IsUint32(), "forth parameter to container.run must be the timeout");
    }

    v8::String::Utf8Value func(args[0]->ToString());

    auto runArgValues = napa::v8_helpers::V8ArrayToVector<napa::v8_helpers::Utf8String>(
        isolate,
        v8::Local<v8::Array>::Cast(args[1]));

    std::vector<NapaStringRef> runArgs;
    runArgs.reserve(runArgValues.size());
    for (const auto& val : runArgValues) {
        runArgs.emplace_back(NAPA_STRING_REF_WITH_SIZE(val.Data(), val.Length()));
    }

    auto callback = v8::Local<v8::Function>::Cast(args[2]);

    auto handler = NodeAsyncHandler<napa::Response>::New(
        isolate,
        callback,
        [isolate](const auto& response) { return CreateResponseValues(isolate, response); }
    );

    auto wrap = ObjectWrap::Unwrap<ContainerWrap>(args.Holder());

    if (args.Length() > 3) {
        wrap->_container->Run(
            *func,
            runArgs,
            [handler](napa::Response response) { handler->DispatchCallback(std::move(response)); },
            args[3]->Uint32Value(context).FromJust()); // timeout
    } else {
        wrap->_container->Run(
            *func,
            runArgs,
            [handler](napa::Response response) { handler->DispatchCallback(std::move(response)); });
    }
}

void ContainerWrap::RunSync(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    CHECK_ARG(isolate, args[0]->IsString(), "first parameter to container.runSync must be the function name");
    CHECK_ARG(isolate, args[1]->IsArray(), "second parameter to container.runSync must be the arguments array");

    if (args.Length() > 2) {
        CHECK_ARG(isolate, args[2]->IsUint32(), "third parameter to container.runSync must be the timeout");
    }

    v8::String::Utf8Value func(args[0]->ToString());
    
    auto runArgValues = napa::v8_helpers::V8ArrayToVector<napa::v8_helpers::Utf8String>(
        isolate,
        v8::Local<v8::Array>::Cast(args[1]));

    std::vector<NapaStringRef> runArgs;
    runArgs.reserve(runArgValues.size());
    for (const auto& val : runArgValues) {
        runArgs.emplace_back(NAPA_STRING_REF_WITH_SIZE(val.Data(), val.Length()));
    }

    auto wrap = ObjectWrap::Unwrap<ContainerWrap>(args.Holder());

    napa::Response response;
    if (args.Length() > 2) {
        // Call with provided timeout.
        response = wrap->_container->RunSync(*func, runArgs, args[2]->Uint32Value(context).FromJust());
    } else {
        response = wrap->_container->RunSync(*func, runArgs);
    }

    auto responseValues = CreateResponseValues(isolate, response);

    auto returnObj = v8::Object::New(isolate);

    (void)returnObj->CreateDataProperty(context, MakeV8String(isolate, "code"), responseValues[0]);
    (void)returnObj->CreateDataProperty(context, MakeV8String(isolate, "errorMessage"), responseValues[1]);
    (void)returnObj->CreateDataProperty(context, MakeV8String(isolate, "returnValue"), responseValues[2]);

    args.GetReturnValue().Set(returnObj);
}

void ContainerWrap::RunAll(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    CHECK_ARG(isolate, args[0]->IsString(), "first parameter to container.runAll must be the function name");
    CHECK_ARG(isolate, args[1]->IsArray(), "second parameter to container.runAll must be the arguments array");
    CHECK_ARG(isolate, args[2]->IsFunction(), "third parameter to container.runAll must be the callback");

    v8::String::Utf8Value func(args[0]->ToString());

    auto runArgValues = napa::v8_helpers::V8ArrayToVector<napa::v8_helpers::Utf8String>(
        isolate,
        v8::Local<v8::Array>::Cast(args[1]));

    std::vector<NapaStringRef> runArgs;
    runArgs.reserve(runArgValues.size());
    for (const auto& val : runArgValues) {
        runArgs.emplace_back(NAPA_STRING_REF_WITH_SIZE(val.Data(), val.Length()));
    }

    auto callback = v8::Local<v8::Function>::Cast(args[2]);

    auto handler = NodeAsyncHandler<NapaResponseCode>::New(isolate, callback, [isolate](const auto& responseCode) {
        std::vector<v8::Local<v8::Value>> res;
        res.push_back(v8::Uint32::NewFromUnsigned(isolate, responseCode));
        return res;
    });

    auto wrap = ObjectWrap::Unwrap<ContainerWrap>(args.Holder());

    wrap->_container->RunAll(*func, runArgs, [handler](NapaResponseCode responseCode) {
        handler->DispatchCallback(responseCode);
    });
}

void ContainerWrap::RunAllSync(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    CHECK_ARG(isolate, args[0]->IsString(), "first parameter to container.runAllSync must be the function name");
    CHECK_ARG(isolate, args[1]->IsArray(), "second parameter to container.runAllSync must be the arguments array");

    v8::String::Utf8Value func(args[0]->ToString());

    auto runArgValues = napa::v8_helpers::V8ArrayToVector<napa::v8_helpers::Utf8String>(
        isolate,
        v8::Local<v8::Array>::Cast(args[1]));

    std::vector<NapaStringRef> runArgs;
    runArgs.reserve(runArgValues.size());
    for (const auto& val : runArgValues) {
        runArgs.emplace_back(NAPA_STRING_REF_WITH_SIZE(val.Data(), val.Length()));
    }

    auto wrap = ObjectWrap::Unwrap<ContainerWrap>(args.Holder());

    auto responseCode = wrap->_container->RunAllSync(*func, runArgs);

    args.GetReturnValue().Set(v8::Uint32::NewFromUnsigned(isolate, responseCode));
}

static std::vector<v8::Local<v8::Value>> CreateResponseValues(v8::Isolate* isolate, const napa::Response& response) {

    auto code = v8::Uint32::NewFromUnsigned(isolate, response.code);

    auto errorMessage = MakeV8String(isolate, response.errorMessage);

    v8::Local<v8::Value> returnValue;
    if (response.returnValue.empty()) {
        returnValue = v8::String::Empty(isolate);
    } else {
        // If parse fails, return the string as is.
        returnValue = MakeV8String(isolate, response.returnValue);

        v8::TryCatch tryCatch;
        returnValue = v8::JSON::Parse(isolate, v8::Local<v8::String>::Cast(returnValue)).FromMaybe(returnValue);
    }

    return { code, errorMessage, returnValue };
}
