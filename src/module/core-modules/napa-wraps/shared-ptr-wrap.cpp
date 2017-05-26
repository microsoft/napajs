#include "shared-ptr-wrap.h"

using namespace napa::module;

NAPA_DEFINE_PERSISTENT_CONSTRUCTOR(napa::module::SharedPtrWrap)

void SharedPtrWrap::Init() {
    auto isolate = v8::Isolate::GetCurrent();
    auto constructorTemplate = v8::FunctionTemplate::New(isolate, DefaultConstructorCallback<SharedPtrWrap>);
    constructorTemplate->SetClassName(v8_helpers::MakeV8String(isolate, SharedPtrWrap::exportName));
    constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);

    InitConstructorTemplate<SharedPtrWrap>(constructorTemplate);
    auto constructor = constructorTemplate->GetFunction();
    InitConstructor("<SharedPtrWrap>", constructor);
    NAPA_SET_PERSISTENT_CONSTRUCTOR(exportName, constructor);
}
