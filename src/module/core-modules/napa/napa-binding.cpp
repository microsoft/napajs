#include "napa-binding.h"

#include "metric-wrap.h"
#include "allocator-debugger-wrap.h"
#include "allocator-wrap.h"
#include "call-context-wrap.h"
#include "shared-ptr-wrap.h"
#include "store-wrap.h"
#include "transport-context-wrap-impl.h"
#include "zone-wrap.h"

#include <zone/worker-context.h>

#include <napa.h>
#include <napa-memory.h>
#include <napa/module/binding.h>
#include <napa/module/binding/wraps.h>
#include <napa/providers/logging.h>
#include <napa/providers/metric.h>

using namespace napa;
using namespace napa::module;

static void RegisterBinding(v8::Local<v8::Object> module) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto persistentModule = new v8::Persistent<v8::Object>(isolate, module);
    zone::WorkerContext::Set(zone::WorkerContextItem::NAPA_BINDING, persistentModule);
}

v8::Local<v8::Object> napa::module::binding::GetModule() {
    auto persistentModule = 
        reinterpret_cast<v8::Persistent<v8::Object>*>(
            zone::WorkerContext::Get(zone::WorkerContextItem::NAPA_BINDING));
    
    NAPA_ASSERT(persistentModule != nullptr, "\"napajs\" must be required before napa::module::binding::GetModule() can be called from C++.");
    return v8::Local<v8::Object>::New(v8::Isolate::GetCurrent(), *persistentModule);
}

static void CreateZone(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    auto context = isolate->GetCurrentContext();

    CHECK_ARG(isolate, args[0]->IsString(), "first argument to createZone must be a string");
    v8::String::Utf8Value zoneId(args[0]->ToString());

    std::stringstream ss;
    if ((args.Length() > 1) && (!args[1]->IsUndefined())) {
        CHECK_ARG(isolate, args[1]->IsObject(), "second argument to createZone must be an object");
        auto settingsObj = args[1]->ToObject(context).ToLocalChecked();

        auto settingsMap = napa::v8_helpers::V8ObjectToMap<std::string>(isolate, settingsObj);

        for (const auto& kv : settingsMap) {
            ss << " --" << kv.first << " " << kv.second;
        }
    }

    try {
        auto zoneProxy = std::make_unique<napa::Zone>(*zoneId, ss.str());
        args.GetReturnValue().Set(ZoneWrap::NewInstance(std::move(zoneProxy)));
    } catch (const std::exception& ex) {
        JS_FAIL(isolate, ex.what());
    }
}

static void GetZone(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate, args[0]->IsString(), "first argument to getZone must be a string");
    v8::String::Utf8Value zoneId(args[0]->ToString());

    try {
        auto zoneProxy = napa::Zone::Get(*zoneId);
        args.GetReturnValue().Set(ZoneWrap::NewInstance(std::move(zoneProxy)));
    }
    catch (const std::exception &ex) {
        JS_ASSERT(isolate, false, ex.what());
    }
}

static void GetCurrentZone(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    args.GetReturnValue().Set(ZoneWrap::NewInstance(napa::Zone::GetCurrent()));
}

/////////////////////////////////////////////////////////////////////
/// Store APIs

static void CreateStore(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate, args.Length() == 1, "1 argument of 'id' is required.");
    CHECK_ARG(isolate, args[0]->IsString(), "Argument 'id' must be string.");

    auto id = napa::v8_helpers::V8ValueTo<std::string>(args[0]);
    auto store = napa::store::CreateStore(id.c_str());

    JS_ENSURE(isolate, store != nullptr, "Store with id \"%s\" already exists.", id.c_str());

    args.GetReturnValue().Set(StoreWrap::NewInstance(store));
}

static void GetOrCreateStore(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate, args.Length() == 1, "1 argument of 'id' is required.");
    CHECK_ARG(isolate, args[0]->IsString(), "Argument 'id' must be string.");

    auto id = napa::v8_helpers::V8ValueTo<std::string>(args[0]);
    auto store = napa::store::GetOrCreateStore(id.c_str());

    args.GetReturnValue().Set(StoreWrap::NewInstance(store));
}

static void GetStore(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate, args.Length() == 1, "1 argument of 'id' is required.");
    CHECK_ARG(isolate, args[0]->IsString(), "Argument 'id' must be string.");

    auto id = napa::v8_helpers::V8ValueTo<std::string>(args[0]);
    auto store = napa::store::GetStore(id.c_str());

    if (store != nullptr) {
        args.GetReturnValue().Set(StoreWrap::NewInstance(store));
    }
}

static void GetStoreCount(const v8::FunctionCallbackInfo<v8::Value>& args) {
    args.GetReturnValue().Set(static_cast<uint32_t>(napa::store::GetStoreCount()));
}

static void GetCrtAllocator(const v8::FunctionCallbackInfo<v8::Value>& args) {
    args.GetReturnValue().Set(binding::CreateAllocatorWrap(
        std::shared_ptr<napa::memory::Allocator>(
            &napa::memory::GetCrtAllocator(),
            [](napa::memory::Allocator*){})));
}

static void GetDefaultAllocator(const v8::FunctionCallbackInfo<v8::Value>& args) {
    args.GetReturnValue().Set(binding::CreateAllocatorWrap(
        std::shared_ptr<napa::memory::Allocator>(
            &napa::memory::GetDefaultAllocator(),
            [](napa::memory::Allocator*){})));
}

static void Log(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate, args.Length() == 4, "log accepts exactly 4 arguments (level, section, traceId, message)");
    CHECK_ARG(isolate, args[0]->IsUint32(), "'level' must be a uint32 type that represents the native enum");
    CHECK_ARG(isolate, args[1]->IsString() || args[1]->IsUndefined(), "'section' must be a valid string or undefined");

    auto level = static_cast<napa::providers::LoggingProvider::Verboseness>(args[0]->Uint32Value());

    napa::v8_helpers::Utf8String sectionValue;
    const char* section = "";
    if (!args[1]->IsUndefined()) {
        sectionValue = napa::v8_helpers::V8ValueTo<napa::v8_helpers::Utf8String>(args[1]);
        section = sectionValue.Length() > 0 ? sectionValue.Data() : "";
    }

    auto& logger = napa::providers::GetLoggingProvider();

    // If log is not enabled we can return early.
    if (!logger.IsLogEnabled(section, level)) {
        return;
    }

    CHECK_ARG(isolate, args[2]->IsString() || args[2]->IsUndefined(), "'traceId' must be a valid string or undefined");
    CHECK_ARG(isolate, args[3]->IsString(), "'message' must be a valid string");

    napa::v8_helpers::Utf8String traceIdValue;
    const char* traceId = "";
    if (!args[2]->IsUndefined()) {
        traceIdValue = napa::v8_helpers::V8ValueTo<napa::v8_helpers::Utf8String>(args[2]);
        traceId = traceIdValue.Length() > 0 ? traceIdValue.Data() : "";
    }

    v8::String::Utf8Value message(args[3]->ToString());

    // Get the first frame in user code.
    // The first 2 frames are part of the log.js file.
    auto stackFrame = v8::StackTrace::CurrentStackTrace(isolate, 3)->GetFrame(2);

    v8::String::Utf8Value file(stackFrame->GetScriptName());
    int line = stackFrame->GetLineNumber();

    logger.LogMessage(section, level, traceId, *file, line, *message);
}

void binding::Init(v8::Local<v8::Object> exports, v8::Local<v8::Object> module) {
    // Register napa binding in worker context.
    RegisterBinding(module);

    AllocatorDebuggerWrap::Init();
    AllocatorWrap::Init();
    MetricWrap::Init();
    CallContextWrap::Init();
    SharedPtrWrap::Init();
    StoreWrap::Init();
    TransportContextWrapImpl::Init();
    ZoneWrap::Init();

    NAPA_EXPORT_OBJECTWRAP(exports, "AllocatorDebuggerWrap", AllocatorDebuggerWrap);
    NAPA_EXPORT_OBJECTWRAP(exports, "AllocatorWrap", AllocatorWrap);
    NAPA_EXPORT_OBJECTWRAP(exports, "MetricWrap", MetricWrap);
    NAPA_EXPORT_OBJECTWRAP(exports, "CallContextWrap", CallContextWrap);
    NAPA_EXPORT_OBJECTWRAP(exports, "SharedPtrWrap", SharedPtrWrap);
    NAPA_EXPORT_OBJECTWRAP(exports, "TransportContextWrap", TransportContextWrapImpl);

    NAPA_SET_METHOD(exports, "createZone", CreateZone);
    NAPA_SET_METHOD(exports, "getZone", GetZone);
    NAPA_SET_METHOD(exports, "getCurrentZone", GetCurrentZone);

    NAPA_SET_METHOD(exports, "createStore", CreateStore);
    NAPA_SET_METHOD(exports, "getOrCreateStore", GetOrCreateStore);
    NAPA_SET_METHOD(exports, "getStore", GetStore);
    NAPA_SET_METHOD(exports, "getStoreCount", GetStoreCount);

    NAPA_SET_METHOD(exports, "getCrtAllocator", GetCrtAllocator);
    NAPA_SET_METHOD(exports, "getDefaultAllocator", GetDefaultAllocator);

    NAPA_SET_METHOD(exports, "log", Log);
}
