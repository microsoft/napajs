#include "os.h"

#include <napa-module.h>
#include <napa/module/platform.h>

using namespace napa;
using namespace napa::module;

void os::Init(v8::Local<v8::Object> exports) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto context = isolate->GetCurrentContext();

    (void)exports->CreateDataProperty(context,
                                      v8_helpers::MakeV8String(isolate, "type"),
                                      v8_helpers::MakeV8String(isolate, platform::OS_TYPE));
}