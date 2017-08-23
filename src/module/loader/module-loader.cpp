// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "module-loader.h"

#include "binary-module-loader.h"
#include "core-module-loader.h"
#include "javascript-module-loader.h"
#include "json-module-loader.h"
#include "module-cache.h"
#include "module-loader-helpers.h"
#include "module-resolver.h"

#include <module/core-modules/core-modules.h>
#include <platform/filesystem.h>
#include <utils/debug.h>

// TODO: decouple dependencies between module-loader and zone.
#include <zone/worker-context.h>

#include <napa/log.h>
#include <napa/module.h>

#include <unordered_set>

using namespace napa;
using namespace napa::module;

/// <summary> Implementation of module loader. </summary>
/// <remarks>
/// It has three kinds of core modules.
/// Built-in module:
///     It can be accessed with/without require() such as console, process, but can't be with process.binding().
///     It's cached at both binding and module cache.
///     If javascript core file exists, it overrides binary core module.
/// Binary core module:
///     It can be accessed with only process.binding(), not with require().
///     It's cached at only binding cache.
/// Javascript core module:
///     It exists as Javascript file at './lib' or '../lib' directory.
///     It can be accessed with only require() and cached at only module cache.
///     If javascript core file exists, it overrides binary core module.
/// </remarks>
class ModuleLoader::ModuleLoaderImpl {
public:
    /// <summary> Constructor. </summary>
    ModuleLoaderImpl();

    /// <summary> Bootstrap core modules into module loader. </summary>
    /// <remarks>
    /// Bootstraping must be done after module loader is created and registered into isolate data
    /// because Javascript core modules may call 'require'.
    /// </remarks>
    void Bootstrap();

private:
    /// <summary> Global callback function for require(). </summary>
    /// <param name="args"> V8 argument to return module object. </param>
    static void RequireCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

    /// <summary> Callback to resolve a given module path. </summary>
    /// <param name="args"> Module name. </param>
    static void ResolveCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

    /// <summary> Callback to bind core binary module. </summary>
    /// <param name="args"> Module name. </param>
    static void BindingCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

    /// <summary> It loads a module. </summary>
    /// <param name="path"> Module path called by require(). </param>
    /// <param name="args"> V8 argument to return module object. </param>
    void RequireModule(const char* path,
        const v8::FunctionCallbackInfo<v8::Value>& args);

    /// <summary> It registers a binary core module. </summary>
    /// <param name="name"> Module name. </param>
    /// <param name="isBuiltInModule"> True if it's a built-in module, which doesn't need require() to call. </param>
    /// <param name="initializer"> Module initialization function. </param>
    void LoadBinaryCoreModule(const char* name,
        bool isBuiltInModule,
        const napa::module::ModuleInitializer& initializer);

    /// <summary> It sets up built-in modules at each module's' context. </summary>
    /// <param name="context"> V8 context. </param>
    void SetupBuiltInModules(v8::Local<v8::Context> context);

    /// <summary> It sets up require function. </summary>
    /// <param name="context"> V8 context. </param>
    void SetupRequire(v8::Local<v8::Context> context);

    /// <summary> It adds the extra functions, which access module loader's function, into built-in modules. <summary>
    /// <param name="context"> V8 context. </param>
    /// <remarks> It assumes that calling built-in modules are already loaded into a context. </remarks>
    void DecorateBuiltInModules(v8::Local<v8::Context> context);

    /// <summary> Module cache to avoid module loading overhead. </summary>
    ModuleCache _moduleCache;

    /// <summary> Cache for core binary modules, which can be accessed by process.binding(). </summary>
    ModuleCache _bindingCache;

    /// <summary> Module resolver to resolve module path. </summary>
    ModuleResolver _resolver;

    /// <summary> Built-in module list. </summary>
    std::unordered_set<std::string> _builtInNames;

    /// <summary> Module loaders. </summary>
    std::array<std::unique_ptr<ModuleFileLoader>, static_cast<size_t>(ModuleType::END_OF_MODULE_TYPE)> _loaders;
};

void ModuleLoader::CreateModuleLoader() {
    auto moduleLoader = reinterpret_cast<ModuleLoader*>(zone::WorkerContext::Get(zone::WorkerContextItem::MODULE_LOADER));
    if (moduleLoader == nullptr) {
        moduleLoader = new ModuleLoader();
        zone::WorkerContext::Set(zone::WorkerContextItem::MODULE_LOADER, moduleLoader);

        // Now, Javascript core module's 'require' can find module loader instance correctly.
        moduleLoader->_impl->Bootstrap();
    }
    NAPA_DEBUG("ModuleLoader", "Module loader is created successfully.");
}

ModuleLoader::ModuleLoader() :
    _impl(std::make_unique<ModuleLoader::ModuleLoaderImpl>()) {}

ModuleLoader::~ModuleLoader() = default;

ModuleLoader::ModuleLoaderImpl::ModuleLoaderImpl() {
    auto builtInModulesSetter = [this](v8::Local<v8::Context> context) {
        SetupRequire(context);
        SetupBuiltInModules(context);
    };

    // Set up module loaders for each module type.
    _loaders = {{nullptr,
        std::make_unique<CoreModuleLoader>(builtInModulesSetter, _moduleCache, _bindingCache),
        std::make_unique<JavascriptModuleLoader>(builtInModulesSetter, _moduleCache),
        std::make_unique<JsonModuleLoader>(),
        std::make_unique<BinaryModuleLoader>(builtInModulesSetter)}};
}

void ModuleLoader::ModuleLoaderImpl::Bootstrap() {
    // Set up top-level context.
    module_loader_helpers::SetupTopLevelContext();

    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    auto context = isolate->GetCurrentContext();

    // 'require' needs to be available in top-level context before core-module is loaded.
    SetupRequire(context);

    // Initialize core modules listed in core-modules.h.
    INITIALIZE_CORE_MODULES(LoadBinaryCoreModule)
    NAPA_DEBUG("ModuleLoader", "Binary core modules are loaded.");

    // Set up built-in modules from binaries.
    SetupBuiltInModules(context);

    // Load core module information from 'core-modules.json'.
    auto coreModuleInfos = module_loader_helpers::ReadCoreModulesJson();
    for (auto& info : coreModuleInfos) {
        _resolver.SetAsCoreModule(info.name.c_str());

        if (info.isBuiltIn) {
            _builtInNames.emplace(std::move(info.name));
        }
    }

    auto& coreModuleLoader = _loaders[static_cast<size_t>(ModuleType::CORE)];

    // Override built-in modules with javascript file if exists.
    for (const auto& name : _builtInNames) {
        v8::Local<v8::Object> module;
        if (coreModuleLoader->TryGet(name, v8::Local<v8::Value>(), module)) {
            // If javascript core module exists, replace the existing one.
            _moduleCache.Upsert(name, module);
            (void)context->Global()->Set(context,
                v8_helpers::MakeV8String(isolate, name),
                module);
        }
    }
    NAPA_DEBUG("ModuleLoader", "JavaScript core modules are loaded.");
}

void ModuleLoader::ModuleLoaderImpl::RequireCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate,
        args.Length() == 1 || args.Length() == 2 || args[0]->IsString(),
        "Invalid arguments");

    auto moduleLoader = reinterpret_cast<ModuleLoader*>(zone::WorkerContext::Get(zone::WorkerContextItem::MODULE_LOADER));
    JS_ENSURE(isolate, moduleLoader != nullptr, "Module loader is not initialized");

    v8::String::Utf8Value path(args[0]);
    moduleLoader->_impl->RequireModule(*path, args);
}

void ModuleLoader::ModuleLoaderImpl::ResolveCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate, args.Length() == 1 && args[0]->IsString(), "Invalid arguments");

    auto moduleLoader = reinterpret_cast<ModuleLoader*>(zone::WorkerContext::Get(zone::WorkerContextItem::MODULE_LOADER));
    JS_ENSURE(isolate, moduleLoader != nullptr, "Module loader is not initialized");

    v8::String::Utf8Value path(args[0]);
    auto contextDir = module_loader_helpers::GetCurrentContextDirectory();

    auto moduleInfo = moduleLoader->_impl->_resolver.Resolve(*path, contextDir.c_str());
    JS_ENSURE(isolate, moduleInfo.type != ModuleType::NONE, "Cannot find module \"%s\"", *path);

    args.GetReturnValue().Set(v8_helpers::MakeV8String(isolate, moduleInfo.fullPath));
}

void ModuleLoader::ModuleLoaderImpl::BindingCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate, args.Length() == 1 && args[0]->IsString(), "Invalid arguments");

    auto moduleLoader = reinterpret_cast<ModuleLoader*>(zone::WorkerContext::Get(zone::WorkerContextItem::MODULE_LOADER));
    JS_ENSURE(isolate, moduleLoader != nullptr, "Module loader is not initialized");

    v8::String::Utf8Value name(args[0]);
    v8::Local<v8::Object> module;

    if (moduleLoader->_impl->_bindingCache.TryGet(*name, module)) {
        args.GetReturnValue().Set(module);
        return;
    }

    LOG_WARNING("ModuleLoader", "process.binding for \"%s\" does not exist.", *name);
    args.GetReturnValue().SetUndefined();
}

void ModuleLoader::ModuleLoaderImpl::RequireModule(const char* path, const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (path == nullptr) {
        args.GetReturnValue().SetUndefined();
        return;
    }

    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    // Set optional argument for module file loader.
    auto arg = args.Length() == 1 ? v8::Local<v8::Value>() : args[1];
    bool fromContent = !arg.IsEmpty() && arg->IsString();

    // If require is called with a module receiver, use module.filename to deduce context directory.
    std::string contextDir;
    if (!args.Holder().IsEmpty()) {
        contextDir = module_loader_helpers::GetModuleDirectory(args.Holder());
    }

    if (contextDir.empty()) {
        contextDir = module_loader_helpers::GetCurrentContextDirectory();
    }

    ModuleInfo moduleInfo;
    if (fromContent) {
        moduleInfo = ModuleInfo{
            ModuleType::JAVASCRIPT,
            (filesystem::Path(contextDir) / path).Normalize().String(), // Module id
            "" // No package.json
        };
    } else {
        moduleInfo = _resolver.Resolve(path, contextDir.c_str());
        if (moduleInfo.type == ModuleType::NONE) {
            NAPA_DEBUG("ModuleLoader", "Cannot resolve module path \"%s\".", path);
            args.GetReturnValue().SetUndefined();
            return;
        }
    }

    v8::Local<v8::Object> module;

    // Module from content script is not cached.
    if (!fromContent) {
        if (_moduleCache.TryGet(moduleInfo.fullPath, module)) {
            NAPA_DEBUG("ModuleLoader", "Retrieved module from cache: \"%s\".", path);
            args.GetReturnValue().Set(module);
            return;
        }
    }

    auto& loader = _loaders[static_cast<size_t>(moduleInfo.type)];
    JS_ENSURE(isolate, loader != nullptr, "No proper module loader is defined");

    auto succeeded = loader->TryGet(moduleInfo.fullPath, arg, module);
    if (!succeeded) {
        NAPA_DEBUG("ModuleLoader", "Cannot load module \"%s\".", path);
        args.GetReturnValue().SetUndefined();
        return;
    }

    NAPA_DEBUG("ModuleLoader", "Loaded module from file (first time): \"%s\".", path);

    if (!fromContent) {
        _moduleCache.Upsert(moduleInfo.fullPath, module);
    }
    args.GetReturnValue().Set(module);
}

void ModuleLoader::ModuleLoaderImpl::LoadBinaryCoreModule(
    const char* name,
    bool isBuiltInModule,
    const napa::module::ModuleInitializer& initializer) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto context = isolate->GetCurrentContext();

    auto moduleContext = v8::Context::New(isolate);
    NAPA_ASSERT(!moduleContext.IsEmpty(), "Can't set up context for core modules");

    // We set an empty security token so callee can access caller's context.
    moduleContext->SetSecurityToken(v8::Undefined(isolate));
    v8::Context::Scope contextScope(moduleContext);

    module_loader_helpers::SetupModuleContext(context, moduleContext, module_loader_helpers::GetNapaDllPath());

    // Put it into module resolver to prevent from resolving as user module.
    _resolver.SetAsCoreModule(name);

    auto module = module_loader_helpers::ExportModule(moduleContext->Global(), initializer);

    if (isBuiltInModule) {
        _builtInNames.emplace(name);

        // Put core module into cache.
        // This makes the same behavior with node.js, i.e. it must be loaded by 'require'.
        _moduleCache.Upsert(name, module);
    } else {
        // Put into binding cache.
        // It must be accessed by process.binding(), not by require().
        _bindingCache.Upsert(name, module);
    }
}

void ModuleLoader::ModuleLoaderImpl::SetupBuiltInModules(v8::Local<v8::Context> context) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    // Assume that built-in modules are already cached.
    for (const auto& name : _builtInNames) {
        v8::Local<v8::Object> module;
        if (_moduleCache.TryGet(name, module)) {
            (void)context->Global()->CreateDataProperty(context,
                v8_helpers::MakeV8String(isolate, name),
                module);
        }
    }

    // Can decorate a context after loading up built-in modules.
    DecorateBuiltInModules(context);
}

void ModuleLoader::ModuleLoaderImpl::SetupRequire(v8::Local<v8::Context> context) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    // Set up require().
    auto requireFunctionTemplate = v8::FunctionTemplate::New(isolate, RequireCallback);
    (void)context->Global()->CreateDataProperty(context,
        v8_helpers::MakeV8String(isolate, "require"),
        requireFunctionTemplate->GetFunction());

    // Set up require.resolve().
    auto require = context
                       ->Global()
                       ->Get(
                           context,
                           v8_helpers::MakeV8String(isolate, "require"))
                       .ToLocalChecked()
                       ->ToObject();
    auto resolveFunctionTemplate = v8::FunctionTemplate::New(isolate, ResolveCallback);
    (void)require->CreateDataProperty(context,
        v8_helpers::MakeV8String(isolate, "resolve"),
        resolveFunctionTemplate->GetFunction());
}

// If we have more decorations, move them out from this class.
void ModuleLoader::ModuleLoaderImpl::DecorateBuiltInModules(v8::Local<v8::Context> context) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    // Add process.binding()
    auto process = context
                       ->Global()
                       ->Get(context,
                           v8_helpers::MakeV8String(isolate, "process"))
                       .ToLocalChecked()
                       ->ToObject();
    JS_ENSURE(isolate, !process.IsEmpty(), "Process built-in module doesn't exist");

    auto bindingFunctionTemplate = v8::FunctionTemplate::New(isolate, BindingCallback);
    (void)process->CreateDataProperty(context,
        v8_helpers::MakeV8String(isolate, "binding"),
        bindingFunctionTemplate->GetFunction());
}
