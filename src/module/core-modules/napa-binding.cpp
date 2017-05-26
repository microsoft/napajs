#include "napa-binding.h"

#include "napa-wraps/allocator-debugger-wrap.h"
#include "napa-wraps/allocator-wrap.h"
#include "napa-wraps/shared-ptr-wrap.h"
#include "napa-wraps/store-wrap.h"
#include "napa-wraps/transport-context-wrap-impl.h"
#include "napa-wraps/zone-wrap.h"

#include <napa.h>
#include <napa-memory.h>
#include <napa/module/binding/wraps.h>
#include <napa/module/worker-context.h>


using namespace napa;
using namespace napa::module;

static void RegisterBinding(v8::Local<v8::Object> module) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto persistentModule = new v8::Persistent<v8::Object>(isolate, module);
    WorkerContext::Set(WorkerContextItem::NAPA_BINDING, persistentModule);
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
        auto zoneProxy = std::make_unique<napa::ZoneProxy>(*zoneId, ss.str());
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
        auto zoneProxy = napa::ZoneProxy::Get(*zoneId);
        args.GetReturnValue().Set(ZoneWrap::NewInstance(std::move(zoneProxy)));
    }
    catch (const std::exception &ex) {
        JS_ASSERT(isolate, false, ex.what());
    }
}

static void GetCurrentZone(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    args.GetReturnValue().Set(ZoneWrap::NewInstance(napa::ZoneProxy::GetCurrent()));
}

static void FindOrCreateStore(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate, args.Length() == 1, "1 arguments are required for \"findOrCreateStore\".");
    CHECK_ARG(isolate, args[0]->IsString(), "Argument 'id' must be string.");

    auto id = napa::v8_helpers::V8ValueTo<std::string>(args[0]);
    auto store = napa::memory::FindOrCreateStore(id.c_str());

    args.GetReturnValue().Set(StoreWrap::NewInstance(store));
}

static void FindStore(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate, args.Length() == 1, "1 arguments are required for \"findStore\".");
    CHECK_ARG(isolate, args[0]->IsString(), "Argument 'id' must be string.");

    auto id = napa::v8_helpers::V8ValueTo<std::string>(args[0]);
    auto store = napa::memory::FindStore(id.c_str());

    if (store != nullptr) {
        args.GetReturnValue().Set(StoreWrap::NewInstance(store));
    }
}

static void GetStoreCount(const v8::FunctionCallbackInfo<v8::Value>& args) {
    args.GetReturnValue().Set(static_cast<uint32_t>(napa::memory::GetStoreCount()));
}

static void GetCrtAllocator(const v8::FunctionCallbackInfo<v8::Value>& args) {
    args.GetReturnValue().Set(binding::CreateAllocatorWrap(NAPA_MAKE_SHARED<napa::memory::CrtAllocator>()));
}

static void GetDefaultAllocator(const v8::FunctionCallbackInfo<v8::Value>& args) {
    args.GetReturnValue().Set(binding::CreateAllocatorWrap(NAPA_MAKE_SHARED<napa::memory::DefaultAllocator>()));
}

void binding::Init(v8::Local<v8::Object> exports, v8::Local<v8::Object> module) {
    // Register napa binding in worker context.
    RegisterBinding(module);

    AllocatorDebuggerWrap::Init();
    AllocatorWrap::Init();
    SharedPtrWrap::Init();
    StoreWrap::Init();
    TransportContextWrapImpl::Init();
    ZoneWrap::Init();

    NAPA_EXPORT_OBJECTWRAP(exports, "AllocatorDebuggerWrap", AllocatorDebuggerWrap);
    NAPA_EXPORT_OBJECTWRAP(exports, "AllocatorWrap", AllocatorWrap);
    NAPA_EXPORT_OBJECTWRAP(exports, "SharedPtrWrap", SharedPtrWrap);
    NAPA_EXPORT_OBJECTWRAP(exports, "TransportContextWrap", TransportContextWrapImpl);

    NAPA_SET_METHOD(exports, "createZone", CreateZone);
    NAPA_SET_METHOD(exports, "getZone", GetZone);
    NAPA_SET_METHOD(exports, "getCurrentZone", GetCurrentZone);

    NAPA_SET_METHOD(exports, "findOrCreateStore", FindOrCreateStore);
    NAPA_SET_METHOD(exports, "findStore", FindStore);
    NAPA_SET_METHOD(exports, "getStoreCount", GetStoreCount);

    NAPA_SET_METHOD(exports, "getCrtAllocator", GetCrtAllocator);
    NAPA_SET_METHOD(exports, "getDefaultAllocator", GetDefaultAllocator);
}
