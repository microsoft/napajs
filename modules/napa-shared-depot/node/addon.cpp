#include "shared-barrel-wrap.h"
#include "shared-depot.h"

#include <napa-log.h>
#include <napa/shared-wrap.h>

using namespace napa::module;

NAPA_DEFINE_PERSISTENT_SHARED_WRAP_CONSTRUCTOR

SharedDepot& GetSharedDepot() {
    static SharedDepot depot;
    return depot;
}

/// <summary> It returns a shared barrel wrapper instance. </summary>
/// <returns> SharedBarrel instance. </summary>
void Assign(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);

    auto barrel = GetSharedDepot().Assign();
    auto barrelWrap = SharedBarrelWrap::Create(std::move(barrel));

    args.GetReturnValue().Set(barrelWrap);
}

/// <summary> It releases a shared barrel. </summary>
/// <param name="barrel"> SharedBarrel instance created by Assign() or SharedBarrel instance Id. </param>
/// <remarks> Shared barrel owner must call this function after it's not used any more. </remarks>
void Release(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate,
        args.Length() == 1 && (args[0]->IsObject() || args[0]->IsUint32()),
        "Barrel object or barrel id must be given as argument.");

    if (args[0]->IsObject()) {
        auto barrelWrap = NAPA_OBJECTWRAP::Unwrap<SharedBarrelWrap>(args[0]->ToObject());
        NAPA_ASSERT(barrelWrap != nullptr, "Can't unwrap a shared barrel wrapper.");
        GetSharedDepot().Release(barrelWrap->Get());
    } else {
        auto id = args[0]->Uint32Value();
        GetSharedDepot().Release(id);
    }

    args.GetReturnValue().SetUndefined();
}

/// <summary> It finds a shared barrel. </summary>
/// <param name="id"> Shared barral id. </summary>
/// <returns> SharedBarrel wrapper instance if found, otherwise, return null. </summary>
void Find(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate,
        args.Length() == 1 && args[0]->IsUint32(),
        "Barrel Id must be given.");

    auto barrel = GetSharedDepot().Find(args[0]->Uint32Value());
    if (barrel == nullptr) {
        args.GetReturnValue().SetNull();
        return;
    }

    auto barrelWrap = SharedBarrelWrap::Create(std::move(barrel));
    args.GetReturnValue().Set(barrelWrap);
}

/// <summary> It returns the number of assigned shared barrels. </summary>
void Count(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);

    uint32_t count = static_cast<uint32_t>(GetSharedDepot().GetCount());
    args.GetReturnValue().Set(v8::Uint32::NewFromUnsigned(isolate, count));
}

void InitAll(v8::Local<v8::Object> exports) {
    GetSharedDepot();

    NAPA_SHARED_WRAP_INIT();
    SharedBarrelWrap::Init();

    NAPA_SET_METHOD(exports, "assign", Assign);
    NAPA_SET_METHOD(exports, "release", Release);
    NAPA_SET_METHOD(exports, "find", Find);
    NAPA_SET_METHOD(exports, "count", Count);
}

NAPA_MODULE(addon, InitAll);