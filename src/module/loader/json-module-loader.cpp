// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "json-module-loader.h"
#include "module-loader-helpers.h"

#include <napa/v8-helpers.h>

using namespace napa;
using namespace napa::module;

bool JsonModuleLoader::TryGet(const std::string& path, v8::Local<v8::Object>& module) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope scope(isolate);

    auto source = module_loader_helpers::ReadModuleFile(path);
    JS_ENSURE_WITH_RETURN(isolate, !source.IsEmpty(), false, "Can't read JSON module: \"%s\"", path.c_str());

    auto json = v8::JSON::Parse(isolate, source).ToLocalChecked();
    JS_ENSURE_WITH_RETURN(isolate, !json.IsEmpty(), false, "Can't parse JSON from \"%s\"", path.c_str());

    module = scope.Escape(json->ToObject(isolate->GetCurrentContext()).ToLocalChecked());
    return true;
}
