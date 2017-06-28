#include "binary-module-loader.h"
#include "module-loader-helpers.h"

#include <napa/v8-helpers.h>

#include <boost/dll.hpp>

using namespace napa;
using namespace napa::module;

BinaryModuleLoader::BinaryModuleLoader(BuiltInModulesSetter builtInModulesSetter)
    : _builtInModulesSetter(std::move(builtInModulesSetter)) {}

bool BinaryModuleLoader::TryGet(const std::string& path, v8::Local<v8::Object>& module) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope scope(isolate);

    auto napaModule = boost::dll::import<NapaModule>(path, napa::module::NAPA_MODULE_EXPORT);
    JS_ENSURE_WITH_RETURN(isolate,
                          napaModule != nullptr,
                          false,
                          "Can't import napa module: \"%s\"", 
                          path.c_str());

    JS_ENSURE_WITH_RETURN(isolate,
                          napaModule->version == MODULE_VERSION,
                          false,
                          "Module version is not compatible: \"%s\"",
                          path.c_str());

    // Since boost::dll unload dll when a reference object is gone, keep an instance into local store.
    _modules.push_back(napaModule);

    auto context = isolate->GetCurrentContext();

    auto moduleContext = v8::Context::New(isolate);
    JS_ENSURE_WITH_RETURN(isolate, !moduleContext.IsEmpty(), false, "Can't create module context for \"%s\"", path.c_str());

    // We set an empty security token so callee can access caller's context.
    moduleContext->SetSecurityToken(v8::Undefined(isolate));
    v8::Context::Scope contextScope(moduleContext);

    module_loader_helpers::SetupModuleContext(context, moduleContext, path);

    _builtInModulesSetter(moduleContext);

    module = scope.Escape(module_loader_helpers::ExportModule(moduleContext->Global(), napaModule->initializer));
    return true;
}