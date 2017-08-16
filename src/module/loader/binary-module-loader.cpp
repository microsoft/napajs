// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "binary-module-loader.h"
#include "module-loader-helpers.h"

#include <napa/v8-helpers.h>

#include <platform/dll.h>

using namespace napa;
using namespace napa::module;

BinaryModuleLoader::BinaryModuleLoader(BuiltInModulesSetter builtInModulesSetter)
    : _builtInModulesSetter(std::move(builtInModulesSetter)) {}

bool BinaryModuleLoader::TryGet(const std::string& path, v8::Local<v8::Object>& module) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope scope(isolate);

    auto library = std::make_shared<dll::SharedLibrary>(path);
    auto napaModule = library->Import<NapaModule>(napa::module::NAPA_MODULE_EXPORT);

    JS_ENSURE_WITH_RETURN(isolate, napaModule != nullptr, false, "Can't import napa module: \"%s\"", path.c_str());

    JS_ENSURE_WITH_RETURN(isolate,
                          napaModule->version == MODULE_VERSION,
                          false,
                          "Module version is not compatible: \"%s\"",
                          path.c_str());

    _modules.push_back(library);

    auto context = isolate->GetCurrentContext();

    auto moduleContext = v8::Context::New(isolate);
    JS_ENSURE_WITH_RETURN(
        isolate, !moduleContext.IsEmpty(), false, "Can't create module context for \"%s\"", path.c_str());

    // We set an empty security token so callee can access caller's context.
    moduleContext->SetSecurityToken(v8::Undefined(isolate));
    v8::Context::Scope contextScope(moduleContext);

    module_loader_helpers::SetupModuleContext(context, moduleContext, path);

    _builtInModulesSetter(moduleContext);

    module = scope.Escape(module_loader_helpers::ExportModule(moduleContext->Global(), napaModule->initializer));
    return true;
}