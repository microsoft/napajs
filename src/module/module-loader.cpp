#include "module-loader.h"

#include "binary-module-loader.h"
#include "core-module-loader.h"
#include "javascript-module-loader.h"
#include "json-module-loader.h"
#include "module-cache.h"
#include "module-loader-helpers.h"
#include "module-resolver.h"

#include "core-modules/core-modules.h"

// This is not a module extension, so define this macro to use V8 common macros.
#define NAPA_MODULE_EXTENSION
#include <napa-module.h>

#include <napa-log.h>

using namespace napa;
using namespace napa::module;

class ModuleLoader::ModuleLoaderImpl {
public:

    /// <summary> Constructor. </summary>
    ModuleLoaderImpl();

private:

    /// <summary> Global callback function for require(). </summary>
    /// <param name="args"> V8 argument to return module object. </param>
    static void RequireCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

    /// <summary> Callback to resolve a given module path. </summary>
    /// <param name="args"> Module name. </param>
    static void ResolveCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

    /// <summary> It loads a module. </summary>
    /// <param name="path"> Module path called by require(). </param>
    /// <param name="args"> V8 argument to return module object. </param>
    void RequireModule(const char* path,
                       const v8::FunctionCallbackInfo<v8::Value>& args);

    /// <summary> It registers a binary core module. </summary>
    /// <param name="name"> Module name. </param>
    /// <param name="isBuiltIn"> True if it's a built-in module, which doesn't need require() to call. </param>
    /// <param name="initializer"> Module initialization function. </param>
    void LoadBinaryCoreModule(const char* name,
                              bool isBuiltIn,
                              const napa::module::ModuleInitializer& initializer);

    /// <summary> It sets up built-in modules, which doesn't need to be called by require(). </summary>
    /// <param name="context"> V8 context. </param>
    void SetUpBuiltIns(v8::Local<v8::Context> context);

    /// <summary> It sets up require function. </summary>
    void SetUpRequire(v8::Local<v8::Context> context);

    /// <summary> Module cache to avoid module loading overhead. </summary>
    ModuleCache _cache;

    /// <summary> Module resolver to resolve module path. </summary>
    ModuleResolver _resolver;

    /// <summary> Built-in module list. </summary>
    std::vector<std::string> _builtIns;

    /// <summary> Module loaders. </summary>
    std::array<std::unique_ptr<ModuleFileLoader>, static_cast<size_t>(ModuleType::END_OF_MODULE_TYPE)> _loaders;
};

void ModuleLoader::CreateModuleLoader() {
    auto moduleLoader = reinterpret_cast<ModuleLoader*>(IsolateData::Get(IsolateDataId::MODULE_LOADER));
    if (moduleLoader == nullptr) {
        moduleLoader = new ModuleLoader();
        IsolateData::Set(IsolateDataId::MODULE_LOADER, moduleLoader);
    }
}

ModuleLoader::ModuleLoader() : _impl(std::make_unique<ModuleLoader::ModuleLoaderImpl>()) {}

ModuleLoader::~ModuleLoader() = default;

ModuleLoader::ModuleLoaderImpl::ModuleLoaderImpl() {
    auto builtInsSetter = [this](v8::Local<v8::Context> context) {
        SetUpBuiltIns(context);
    };

    // Set up module loaders for each module type.
    _loaders = {{
        nullptr,
        std::make_unique<CoreModuleLoader>(_cache),
        std::make_unique<JavascriptModuleLoader>(builtInsSetter, _cache),
        std::make_unique<JsonModuleLoader>(),
        std::make_unique<BinaryModuleLoader>(builtInsSetter)
    }};

    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto context = isolate->GetCurrentContext();

    // Set the root module path of global context as the script path.
    module_loader_helpers::SetContextModulePath(context->Global());

    // Initialize core modules listed in core-modules.h.
    INITIALIZE_CORE_MODULES(LoadBinaryCoreModule)

    // Set up built-in modules at this context.
    SetUpBuiltIns(context);
}

void ModuleLoader::ModuleLoaderImpl::RequireCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate,
        args.Length() == 1 || args.Length() == 2 || args[0]->IsString(),
        "Invalid arguments");

    auto moduleLoader = reinterpret_cast<ModuleLoader*>(IsolateData::Get(IsolateDataId::MODULE_LOADER));
    JS_ASSERT(isolate, moduleLoader != nullptr, "Module loader is not initialized");

    v8::String::Utf8Value path(args[0]);
    moduleLoader->_impl->RequireModule(*path, args);
}

void ModuleLoader::ModuleLoaderImpl::ResolveCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate, args.Length() == 1 && args[0]->IsString(), "Invalid arguments");

    auto moduleLoader = reinterpret_cast<ModuleLoader*>(IsolateData::Get(IsolateDataId::MODULE_LOADER));
    JS_ASSERT(isolate, moduleLoader != nullptr, "Module loader is not initialized");

    v8::String::Utf8Value path(args[0]);
    auto contextDir = module_loader_helpers::GetCurrentContextDirectory();

    auto resolvedPath = moduleLoader->_impl->_resolver.Resolve(*path, contextDir.c_str());
    args.GetReturnValue().Set(v8_helpers::MakeV8String(isolate, resolvedPath.fullPath));
}

void ModuleLoader::ModuleLoaderImpl::RequireModule(const char* path, const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (path == nullptr) {
        args.GetReturnValue().SetUndefined();
        return;
    }

    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto contextDir = module_loader_helpers::GetCurrentContextDirectory();
    auto moduleInfo = _resolver.Resolve(path, contextDir.c_str());
    if (moduleInfo.type == ModuleType::NONE) {
        args.GetReturnValue().SetUndefined();
        return;
    }

    v8::Local<v8::Object> module;
    if (_cache.TryGet(moduleInfo.fullPath, module)) {
        args.GetReturnValue().Set(module);
        return;
    }

    auto& loader = _loaders[static_cast<size_t>(moduleInfo.type)];
    JS_ASSERT(isolate, loader != nullptr, "No proper module loader is defined");

    auto succeeded = loader->TryGet(moduleInfo.fullPath, module);
    if (!succeeded) {
        args.GetReturnValue().SetUndefined();
        return;
    }

    _cache.Insert(moduleInfo.fullPath, module);
    args.GetReturnValue().Set(module);
}

void ModuleLoader::ModuleLoaderImpl::LoadBinaryCoreModule(
        const char* name,
        bool isBuiltIn,
        const napa::module::ModuleInitializer& initializer) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto context = module_loader_helpers::SetUpModuleContext(std::string());
    NAPA_ASSERT(!context.IsEmpty(), "Can't set up context for core modules");

    // We set an empty security token so callee can access caller's context.
    context->SetSecurityToken(v8::Undefined(isolate));
    v8::Context::Scope contextScope(context);

    auto module = module_loader_helpers::ExportModule(context->Global(), initializer);

    // Put core module into cache.
    // This makes the same behavior with node.js, i.e. it must be loaded by 'require'.
    _cache.Insert(name, module);

    // Put it into module resolver to prevent from resolving as user module.
    _resolver.SetAsCoreModule(name);

    if (isBuiltIn) {
        _builtIns.emplace_back(name);
    }
}

void ModuleLoader::ModuleLoaderImpl::SetUpBuiltIns(v8::Local<v8::Context> context) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    SetUpRequire(context);

    auto& coreModuleLoader = _loaders[static_cast<size_t>(ModuleType::CORE)];

    // Assume that built-in modules are already cached.
    for (const auto& builtIn : _builtIns) {
        v8::Local<v8::Object> module;
        if (coreModuleLoader->TryGet(builtIn, module)) {
            (void)context->Global()->CreateDataProperty(context,
                                                        v8_helpers::MakeV8String(isolate, builtIn),
                                                        module);
        }
    }
}

void ModuleLoader::ModuleLoaderImpl::SetUpRequire(v8::Local<v8::Context> context) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    // Set up require().
    auto requireFunctionTemplate = v8::FunctionTemplate::New(isolate, RequireCallback);
    (void)context->Global()->CreateDataProperty(context,
                                                v8_helpers::MakeV8String(isolate, "require"),
                                                requireFunctionTemplate->GetFunction());

    // Set up require.resolve().
    auto require = context->Global()->Get(context,
                                          v8_helpers::MakeV8String(isolate, "require")).ToLocalChecked()->ToObject();
    auto resolveFunctionTemplate = v8::FunctionTemplate::New(isolate, ResolveCallback);
    (void)require->CreateDataProperty(context,
                                      v8_helpers::MakeV8String(isolate, "resolve"),
                                      resolveFunctionTemplate->GetFunction());
}