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
                          "Can't import napa module: " + path,
                          false);
    JS_ENSURE_WITH_RETURN(isolate,
                          napaModule->version == MODULE_VERSION,
                          "Module version is not compatible: " + path,
                          false);

    // Since boost::dll unload dll when a reference object is gone, keep an instance into local store.
    _modules.push_back(napaModule);

    auto context = module_loader_helpers::SetUpModuleContext(path);
    JS_ENSURE_WITH_RETURN(isolate, !context.IsEmpty(), "Can't create module context for " + path, false);

    // We set an empty security token so callee can access caller's context.
    context->SetSecurityToken(v8::Undefined(isolate));
    v8::Context::Scope contextScope(context);

    _builtInModulesSetter(context);

    module = scope.Escape(module_loader_helpers::ExportModule(context->Global(), napaModule->initializer));
    return true;
}