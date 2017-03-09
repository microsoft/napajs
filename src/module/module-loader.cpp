#include "module-loader.h"

#include "built-in/built-in-modules.h"
#include "core/core-modules.h"
#include "core/file-system-helpers.h"

#include <napa/v8-helpers.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/dll.hpp>

#include <sstream>

using namespace napa;
using namespace napa::module;

namespace {
    const std::string NAPA_MODULE_EXTENSION = ".napa";
    const std::string JAVASCRIPT_MODULE_EXTENSION = ".js";

    std::string _moduleRootDirectory = boost::dll::this_line_location().parent_path().string();
}   // End of anonymous namespace.

class ModuleLoader::ModuleLoaderImpl {
public:

    /// <summary>
    /// Load a module. It first tries to load native module with extension "napa".
    /// Then, it tries to load javascript module.
    /// </summary>
    /// <param name="moduleName"> Module name called by require(). </param>
    /// <param name="args"> V8 argument to return module object. </param>
    void RequireModule(const std::string& moduleName,
                        const v8::FunctionCallbackInfo<v8::Value>& args);

    /// <summary> Create module object. </summary>
    /// <param name="moduleFileName"> Path of module file. For built-in module, nullptr should be provided. </param>
    v8::Local<v8::ObjectTemplate> CreateModuleObject(const boost::filesystem::path* moduleFileName);

    /// <summary> Cache a module with path. </summary>
    /// <param name="modulePath"> The module name path. </param>
    /// <param name="exports"> V8 representative javascript object to be cached. </param>
    void CacheModule(const std::string& modulePath,
                     v8::Local<v8::Object> exports);

private:

    /// <summary> An helper method returns the module path string without any path manipulations. </summary>
    /// <param name="moduleName"> Module name called by require(). </param>
    std::string GetRawModulePath(const std::string& moduleName) const;

    /// <summary> An helper method that checks if the module name is actually a built-in module and loads it if it is. </summary>
    /// <param name="moduleName"> Module name called by require(). </param>
    /// <param name="args"> V8 argument to return module object. </param>
    /// <returns> True if the built-in module was loaded, false otherwise. </returns>
    bool TryGetCoreModule(const std::string& moduleName, 
                          const v8::FunctionCallbackInfo<v8::Value>& args) const;

    /// <summary> An helper that checks if the module name contains path information. </summary>
    /// <param name="moduleName"> Module name called by require(). </param>
    /// <returns> True if the module name contains path information, false otherwise. </returns>
    bool IsPathProvided(const std::string& moduleName) const;

    /// <summary> An helper that check if the module name contains the relative path. </summary>
    /// <param name="moduleName"> Module name called by require(). </param>
    /// <returns> True if the module name contains the relative path, false otherwise. </returns>
    bool IsRelativePath(const std::string& moduleName) const;

    /// <summary> An helper that checks if the module name contains the absoulte path. </summary>
    /// <param name="moduleName"> Module name called by require(). </param>
    /// <returns> True if the module name contains the absolute path, false otherwise. </returns>
    bool IsAbsolutePath(const std::string& moduleName) const;

    /// <summary> An helper that to load a module from cache using path string. </summary>
    /// <param name="modulePath"> Full path of javascript or native addon. </param>
    /// <param name="args"> V8 argument to return module object. </param>
    /// <returns> True if the module exists in cache, false otherwise. </returns>
    bool TryResolveFromCache(const std::string& modulePath,
                             const v8::FunctionCallbackInfo<v8::Value>& args) const;

    /// <summary> An helper that to load a module from cache. </summary>
    /// <param name="modulePath"> Full path of javascript or native addon. </param>
    /// <param name="args"> V8 argument to return module object. </param>
    /// <returns> True if the module exists in cache, false otherwise. </returns>
    bool TryResolveFromCache(const boost::filesystem::path& modulePath,
                             const v8::FunctionCallbackInfo<v8::Value>& args) const;

    /// <summary> An helper that to load a module from cache. </summary>
    /// <param name="modulePath"> Full path of javascript or native addon. </param>
    /// <param name="cachedModule"> The resolved cached module if successful. </param>
    /// <returns> True if the module exists in cache, false otherwise. </returns>
    bool TryResolveFromCache(const boost::filesystem::path& modulePath,
                             v8::Local<v8::Object>& cachedModule) const;

    /// <summary> Loads the module from the file path. </summary>
    /// <param name="moduleName"> Module name called by require(). </param>
    /// <param name="modulePath"> Full path of javascript or native addon. </param>
    /// <param name="args"> V8 argument to return module object. </param>
    /// <returns> True if the module was loaded, false otherwise. </returns>
    bool LoadModuleFromFile(const boost::filesystem::path& modulePath,
                            const v8::FunctionCallbackInfo<v8::Value>& args);

    /// <summary> Tries to search for the module by climbing the directory hierarchy according to node 
    /// resolve algorithm. </summary>
    /// <param name="moduleName"> Module name called by require(). </param>
    /// <param name="currentDir"> Current directory. </param>
    /// <param name="args"> V8 argument to return module object. </param>
    void ResolveModuleRecursive(const std::string& moduleName,
                                const boost::filesystem::path& currentDir,
                                const v8::FunctionCallbackInfo<v8::Value>& args);

    /// <summary> Load native module. </summary>
    /// <param name="modulePath"> Full path of javascript or native addon. </param>
    /// <param name="args"> V8 argument to return module object. </param>
    void LoadNapaModule(const boost::filesystem::path& modulePath,
                        const v8::FunctionCallbackInfo<v8::Value>& args);

    /// <summary> Load javascript module from string. </summary>
    /// <param name="source"> Javascript string. </param>
    /// <param name="modulePath"> Full path of javascript or native addon. </param>
    /// <param name="args"> V8 argument to return module object. </param>
    void LoadJavascriptModule(v8::Local<v8::String> source,
                              const boost::filesystem::path& modulePath,
                              const v8::FunctionCallbackInfo<v8::Value>& args,
                              bool shouldCache = true);

    /// <summary> Load javascript module from file. </summary>
    /// <param name="moduoePath"> Fjull path of javascript or native addon. </param>
    /// <param name="args"> V8 argument to return module object. </param>
    void LoadJavascriptModule(const boost::filesystem::path& modulePath,
                              const v8::FunctionCallbackInfo<v8::Value>& args);

    /// <summary> Exports loaded module. </summary>
    /// <param name="moduleObject"> Loaded V8 representative javascript object. </param>
    /// <param name="registerModule"> Callback function to register native addon. </param>
    v8::Local<v8::Object> ExportModule(v8::Local<v8::Object> moduleObject,
                                       const napa::module::ModuleInitializer& moduleInitializer);

    using PersistentModule = v8::Persistent<v8::Object, v8::CopyablePersistentTraits<v8::Object>>;
    using PersistentModuleCache = std::unordered_map<std::string, PersistentModule>;

    /// <summary> Module cache to avoid module loading overhead. </summary>
    PersistentModuleCache _moduleCache;

    /// <summary> Keep dll loaded. </summary>
    std::vector<boost::shared_ptr<ModuleInfo>> _moduleInfos;
};

ModuleLoader::ModuleLoader() : _impl(std::make_unique<ModuleLoader::ModuleLoaderImpl>()) {}

const std::string& ModuleLoader::GetModuleRootDirectory() {
    return _moduleRootDirectory;
}

void ModuleLoader::InitializeBuiltIns(v8::Local<v8::Context> context) {
    auto isolate = v8::Isolate::GetCurrent();

    auto functionTemplate = v8::FunctionTemplate::New(isolate, RequireCallback);
    (void)context->Global()->CreateDataProperty(isolate->GetCurrentContext(),
                                                v8_helpers::MakeV8String(isolate, "require"),
                                                functionTemplate->GetFunction());

    // Initialize built-in modules listed in built-in.h.
    // Built-ins are modules that appear in global scope without 'require'.
    INITIALIZE_BUILT_IN_MODULES(context)
}

void ModuleLoader::RegisterCoreModule(const char* moduleName,
                                      const std::function<void (v8::Local<v8::Object>)>& initializer) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto moduleLoader = reinterpret_cast<ModuleLoader*>(IsolateData::Get(IsolateDataId::MODULE_LOADER));
    if (moduleLoader == nullptr) {
        moduleLoader = CreateAndInitialize(false);
    }

    // Create module context following the node module algorithm.
    auto moduleObject = moduleLoader->_impl->CreateModuleObject(nullptr);

    // Create a sandbox to load javascript module.
    // We set an empty security token so callee can access caller's context.
    auto moduleContext = v8::Context::New(isolate, nullptr, moduleObject);
    moduleContext->SetSecurityToken(v8::Undefined(isolate));
    v8::Context::Scope contextScope(moduleContext);

    auto module = moduleContext->Global()->Get(v8_helpers::MakeV8String(isolate, "module"))->ToObject();
    auto exports = module->Get(v8_helpers::MakeV8String(isolate, "exports"))->ToObject();
    initializer(exports);

    // Put built-in module into cache.
    // This makes the same behavior with node.js, i.e. it must be loaded by 'require'.
    moduleLoader->_impl->CacheModule(moduleName, exports);
}

ModuleLoader* ModuleLoader::CreateAndInitialize(bool fromJS) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto context = isolate->GetCurrentContext();

    // Set the root module path of global context as the script path.
    bool setContextPath = false;
    if (fromJS) {
        auto scriptName =
            v8::StackTrace::CurrentStackTrace(isolate, 1, v8::StackTrace::kScriptName)->GetFrame(0)->GetScriptName();

        if (!scriptName.IsEmpty()) {
            // script comes from a file.
            boost::filesystem::path moduleBasePath(*v8::String::Utf8Value(scriptName));
            if (moduleBasePath.is_relative()) {
                moduleBasePath = (boost::filesystem::current_path() / moduleBasePath).normalize().make_preferred();
            }

            (void)context->Global()->CreateDataProperty(context,
                v8_helpers::MakeV8String(isolate, "__dirname"),
                v8_helpers::MakeV8String(isolate, moduleBasePath.parent_path().string().c_str()));
            (void)context->Global()->CreateDataProperty(context,
                v8_helpers::MakeV8String(isolate, "__filename"),
                v8_helpers::MakeV8String(isolate, moduleBasePath.string().c_str()));

            setContextPath = true;
        }
    }

    if (!setContextPath) {
        // script comes from a string.
        (void)context->Global()->CreateDataProperty(context,
            v8_helpers::MakeV8String(isolate, "__dirname"),
            v8_helpers::MakeV8String(isolate, _moduleRootDirectory.c_str()));
        (void)context->Global()->CreateDataProperty(context,
            v8_helpers::MakeV8String(isolate, "__filename"),
            v8::Null(isolate));
    }

    auto moduleLoader = new ModuleLoader();
    IsolateData::Set(IsolateDataId::MODULE_LOADER, moduleLoader);

    // Initialize core modules listed in core.h. They are called by require().
    INITIALIZE_CORE_MODULES(ModuleLoader::RegisterCoreModule)

    return moduleLoader;
}

void ModuleLoader::RequireCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate,
        args.Length() == 1 || args.Length() == 2 || args[0]->IsString(),
        "Invalid arguments");

    auto moduleLoader = reinterpret_cast<ModuleLoader*>(IsolateData::Get(IsolateDataId::MODULE_LOADER));
    if (moduleLoader == nullptr) {
        moduleLoader = CreateAndInitialize(true);
    }

    v8::String::Utf8Value modulePath(args[0]);
    moduleLoader->_impl->RequireModule(*modulePath, args);
}

std::string ModuleLoader::GetCurrentDirectory() {
    return GetContextDirectory(v8::Isolate::GetCurrent()->GetCurrentContext());
}

std::string ModuleLoader::GetCallingDirectory() {
    return GetContextDirectory(v8::Isolate::GetCurrent()->GetCallingContext());
}

std::string ModuleLoader::GetContextDirectory(const v8::Local<v8::Context>& context) {
    auto contextObject = context->Global();
    auto dirPropertyName = v8_helpers::MakeV8String(context->GetIsolate(), "__dirname");

    if (contextObject.IsEmpty() 
        || contextObject->IsNull()
        || !contextObject->Has(dirPropertyName)) {
        return _moduleRootDirectory;
    }

    v8::String::Utf8Value callingPath(contextObject->Get(dirPropertyName));
    return std::string(*callingPath);
}

void ModuleLoader::ModuleLoaderImpl::RequireModule(const std::string& moduleName,
                                                   const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (moduleName.empty()) {
        args.GetReturnValue().SetUndefined();
        return;
    }

    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    // If the source JS script is provided on the require statement, i.e. require('feature', 'exports.print = function () { return 1; }')
    // then we give it first priority as it overrides the default behavior.
    if (args[1]->IsString()) {
        boost::filesystem::path modulePath(GetRawModulePath(moduleName));
        modulePath = modulePath.normalize().make_preferred();
        LoadJavascriptModule(args[1]->ToString(), modulePath, args, false);
        return;
    }

    if (TryGetCoreModule(moduleName, args)) {
        return;
    }

    // First chance resolution from cache using non normalized path
    auto rawModulePath = GetRawModulePath(moduleName);
    if (TryResolveFromCache(rawModulePath, args)) {
        return;
    }

    // Normalize the module path, removes './' and '../' and unify separators (convert '/' to '\\' in windows).
    boost::filesystem::path modulePath(rawModulePath);
    modulePath.normalize().make_preferred();

    // Second chance resolution from cache using normalized path
    if (TryResolveFromCache(modulePath, args)) {
        return;
    }

    if (LoadModuleFromFile(modulePath, args)) {
        v8::Local<v8::Object> cachedModule;
        if (TryResolveFromCache(modulePath, cachedModule)) {
            // Add the raw module path to the cache for first chance cache hits
            CacheModule(rawModulePath, cachedModule);
        }

        return;
    }

    // If the module name is given with path information, then the resolve algorithm
    // only looks up the given path and does not search up the folder hierarchy.
    if (IsPathProvided(moduleName)) {
        args.GetReturnValue().SetUndefined();

        std::ostringstream oss;
        oss << "Cannot find module '" << moduleName << "'";
        isolate->ThrowException(v8::Exception::Error(v8_helpers::MakeV8String(isolate, oss.str().c_str())));
        return;
    }

    boost::filesystem::path moduleFindPath(GetCallingDirectory() + "\\" + moduleName);
    ResolveModuleRecursive(moduleName, moduleFindPath.parent_path(), args);
}

void ModuleLoader::ModuleLoaderImpl::CacheModule(const std::string& modulePath,
                                                 v8::Local<v8::Object> exports) {
    if (exports.IsEmpty()) {
        return;
    }

    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    PersistentModule cached(isolate, exports);

    auto iter = _moduleCache.find(modulePath);
    if (iter != _moduleCache.end()) {
        // If exists, it's unfinished module to prevent cycle, so replace it with new one.
        PersistentModule unfinished = iter->second;
        unfinished.Reset();
        iter->second = cached;
    } else {
        _moduleCache.emplace(modulePath, cached);
    }
}

v8::Local<v8::ObjectTemplate> ModuleLoader::ModuleLoaderImpl::CreateModuleObject(
        const boost::filesystem::path* moduleFileName) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope scope(isolate);

    auto context = isolate->GetCurrentContext();

    auto module = v8::ObjectTemplate::New(isolate);
    auto exports = v8::Object::New(isolate);

    (void)exports->CreateDataProperty(context,
                                      v8_helpers::MakeV8String(isolate, "exports"),
                                      v8::Object::New(isolate));
    
    module->Set(v8_helpers::MakeV8String(isolate, "module"), exports);
    module->Set(v8_helpers::MakeV8String(isolate, "exports"),
                exports->Get(v8_helpers::MakeV8String(isolate, "exports"))->ToObject());

    if (moduleFileName != nullptr) {
        module->Set(v8_helpers::MakeV8String(isolate, "__dirname"),
                    v8_helpers::MakeV8String(isolate, moduleFileName->parent_path().string().c_str()));
        module->Set(v8_helpers::MakeV8String(isolate, "__filename"),
                    v8_helpers::MakeV8String(isolate, moduleFileName->string().c_str()));
    } else {
        module->Set(v8_helpers::MakeV8String(isolate, "__dirname"),
                    v8_helpers::MakeV8String(isolate, _moduleRootDirectory.c_str()));
        module->Set(v8_helpers::MakeV8String(isolate, "__filename"),
                    v8::Null(isolate));
    }

    return scope.Escape(module);
}

std::string ModuleLoader::ModuleLoaderImpl::GetRawModulePath(const std::string& moduleName) const {
    if (IsAbsolutePath(moduleName)) {
        return moduleName;
    }

    if (IsRelativePath(moduleName)) {
        return GetCallingDirectory() + "\\" + moduleName;
    }
    
    // For non-absolute require path which doesn't start with '.' or '..', e.g require('module1'),
    // try to load it from MODULE_ROOT_DIR. (Parameter when calling ModuleLoader::SetModuleRootDirectory).
    return _moduleRootDirectory + "\\" + moduleName;
}

bool ModuleLoader::ModuleLoaderImpl::TryGetCoreModule(const std::string& moduleName,
                                                      const v8::FunctionCallbackInfo<v8::Value>& args) const {
    return TryResolveFromCache(moduleName, args);
}

bool ModuleLoader::ModuleLoaderImpl::IsPathProvided(const std::string& moduleName) const {
    return IsRelativePath(moduleName) || IsAbsolutePath(moduleName);
}

bool ModuleLoader::ModuleLoaderImpl::IsRelativePath(const std::string& moduleName) const {
    return boost::starts_with(moduleName, "./") || boost::starts_with(moduleName, "../");
}

bool ModuleLoader::ModuleLoaderImpl::IsAbsolutePath(const std::string& moduleName) const {
    return boost::starts_with(moduleName, "/") || (moduleName.length() > 1 && moduleName[1] == ':');
}

bool ModuleLoader::ModuleLoaderImpl::TryResolveFromCache(const std::string& modulePath,
                                                         const v8::FunctionCallbackInfo<v8::Value>& args) const {
    auto iter = _moduleCache.find(modulePath);
    if (iter == _moduleCache.end()) {
        return false;
    }

    args.GetReturnValue().Set(v8::Local<v8::Object>::New(v8::Isolate::GetCurrent(), iter->second));
    return true;
}

bool ModuleLoader::ModuleLoaderImpl::TryResolveFromCache(const boost::filesystem::path& modulePath,
                                                         const v8::FunctionCallbackInfo<v8::Value>& args) const {
    v8::Local<v8::Object> cachedModule;
    if (TryResolveFromCache(modulePath, cachedModule)) {
        args.GetReturnValue().Set(cachedModule);
        return true;
    }

    return false;
}

bool ModuleLoader::ModuleLoaderImpl::TryResolveFromCache(const boost::filesystem::path& modulePath,
                                                         v8::Local<v8::Object>& cachedModule) const {
    if (modulePath.has_extension()) {
        auto iter = _moduleCache.find(modulePath.string());
        if (iter == _moduleCache.end()) {
            return false;
        }

        cachedModule = v8::Local<v8::Object>::New(v8::Isolate::GetCurrent(), iter->second);
        return true;
    }

    auto iter = _moduleCache.find(modulePath.string());
    if (iter == _moduleCache.end()) {
        iter = _moduleCache.find(boost::filesystem::path(modulePath).replace_extension(JAVASCRIPT_MODULE_EXTENSION).string());
        if (iter == _moduleCache.end()) {
            iter = _moduleCache.find(boost::filesystem::path(modulePath).replace_extension(NAPA_MODULE_EXTENSION).string());
        }
    }

    // Cache miss
    if (iter == _moduleCache.end()) {
        return false;
    }

    cachedModule = v8::Local<v8::Object>::New(v8::Isolate::GetCurrent(), iter->second);
    return true;
}

bool ModuleLoader::ModuleLoaderImpl::LoadModuleFromFile(const boost::filesystem::path& modulePath,
                                                        const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (boost::filesystem::exists(modulePath)) {
        if (modulePath.extension() == JAVASCRIPT_MODULE_EXTENSION) {
            LoadJavascriptModule(modulePath, args);
        } else {
            LoadNapaModule(modulePath, args);
        }
        return true;
    }

    if (!modulePath.has_extension()) {
        boost::filesystem::path path(modulePath);

        path.replace_extension(JAVASCRIPT_MODULE_EXTENSION);
        if (boost::filesystem::exists(path)) {
            LoadJavascriptModule(path, args);
            return true;
        }

        path.replace_extension(NAPA_MODULE_EXTENSION);
        if (boost::filesystem::exists(path)) {
            LoadNapaModule(path, args);
            return true;
        }
    }
    
    return false;
}

void ModuleLoader::ModuleLoaderImpl::ResolveModuleRecursive(const std::string& moduleName,
                                                            const boost::filesystem::path& currentDir,
                                                            const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (currentDir.empty()) { // Recursive stopping condition (error case)
        auto isolate = v8::Isolate::GetCurrent();

        args.GetReturnValue().SetUndefined();

        std::ostringstream oss;
        oss << "Cannot resolve module '" << moduleName << "' using standard node path resolution";
        isolate->ThrowException(v8::Exception::Error(v8_helpers::MakeV8String(isolate, oss.str().c_str())));
        return;
    }

    boost::filesystem::path modulePath = currentDir / "node_modules" / moduleName;
    if (TryResolveFromCache(modulePath, args)) {
        return;
    }

    if (LoadModuleFromFile(modulePath, args)) {
        return;
    }

    ResolveModuleRecursive(moduleName, currentDir.parent_path(), args);
}

void ModuleLoader::ModuleLoaderImpl::LoadNapaModule(const boost::filesystem::path& modulePath,
                                                    const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto moduleInfo = boost::dll::import<ModuleInfo>(modulePath, napa::module::MODULE_INFO_EXPORT);

    if (moduleInfo == nullptr) {
        std::ostringstream oss;
        oss << "Can't get module information of " << modulePath;
        isolate->ThrowException(v8::Exception::Error(v8_helpers::MakeV8String(isolate, oss.str().c_str())));

        args.GetReturnValue().SetUndefined();
        return;
    }

    if (moduleInfo->version != MODULE_VERSION) {
        std::ostringstream oss;
        oss << "This addon version is not supported: " << moduleInfo->version;
        isolate->ThrowException(v8::Exception::Error(v8_helpers::MakeV8String(isolate, oss.str().c_str())));

        args.GetReturnValue().SetUndefined();
        return;
    }

    // Since boost::dll unload dll when a reference object is gone, keep an instance into local store.
    _moduleInfos.push_back(moduleInfo);

    // Create module object following the node module algorithm.
    auto moduleObject = CreateModuleObject(&modulePath);

    // Create a sandbox to load javascript module.
    auto moduleContext = v8::Context::New(isolate, nullptr, moduleObject);

    // When a TerminateExecution request is pending, Context::New returns an empty context
    // Return early, this request is being killed due to timeout
    if (moduleContext.IsEmpty()) {
        args.GetReturnValue().SetUndefined();
        return;
    }

    // We set an empty security token so callee can access caller's context.
    moduleContext->SetSecurityToken(v8::Undefined(isolate));
    v8::Context::Scope contextScope(moduleContext);

    INIT_BUILTIN_MODULES(moduleContext);

    auto exports = ExportModule(moduleContext->Global(), moduleInfo->initializer);
    CacheModule(modulePath.string(), exports);

    args.GetReturnValue().Set(exports);
}

void ModuleLoader::ModuleLoaderImpl::LoadJavascriptModule(v8::Local<v8::String> source,
                                                          const boost::filesystem::path& modulePath,
                                                          const v8::FunctionCallbackInfo<v8::Value>& args,
                                                          bool shouldCache) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    // Create module object following the node module algorithm.
    auto moduleObject = CreateModuleObject(&modulePath);

    // Create a sandbox to load javascript module.
    auto moduleContext = v8::Context::New(isolate, nullptr, moduleObject);

    // When a TerminateExecution request is pending, Context::New returns an empty context
    // Return early, this request is being killed due to timeout
    if (moduleContext.IsEmpty()) {
        args.GetReturnValue().SetUndefined();
        return;
    }

    // We set an empty security token so callee can access caller's context.
    moduleContext->SetSecurityToken(v8::Undefined(isolate));
    v8::Context::Scope contextScope(moduleContext);

    INIT_BUILTIN_MODULES(moduleContext);

    // Set context base directory to load module using relative path.
    // We get nested contexts as call nested functions, eg modules that require other modules.
    // This is how we store multiple working directories correctly.

    // To prevent cycle, cache unloaded module first.
    if (shouldCache) {
        CacheModule(modulePath.string(), ExportModule(moduleContext->Global(), nullptr));
    }

    v8::TryCatch tryCatch;
    {
        auto origin = v8::ScriptOrigin(v8_helpers::MakeV8String(isolate, modulePath.generic_string().c_str()));
        auto script = v8::Script::Compile(source, &origin);
        if (script.IsEmpty() || tryCatch.HasCaught()) {
            tryCatch.ReThrow();

            args.GetReturnValue().SetUndefined();
            return;
        }

        auto run = script->Run();
        if (run.IsEmpty() || tryCatch.HasCaught()) {
            tryCatch.ReThrow();

            args.GetReturnValue().SetUndefined();
            return;
        }
    }

    // Export and cache a loaded version.
    auto exports = ExportModule(moduleContext->Global(), nullptr);
    if (shouldCache) {
        CacheModule(modulePath.string(), exports);
    }

    args.GetReturnValue().Set(exports);
}

void ModuleLoader::ModuleLoaderImpl::LoadJavascriptModule(const boost::filesystem::path& modulePath,
                                                          const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    try {
        auto content = file_system_helpers::ReadFileSync(modulePath.string());
        auto source = v8_helpers::MakeV8String(isolate, content);

        LoadJavascriptModule(source, modulePath, args);
    } catch (const std::exception& ex) {
        isolate->ThrowException(v8::Exception::Error(v8_helpers::MakeV8String(isolate, ex.what())));
        args.GetReturnValue().SetUndefined();
    }
}

v8::Local<v8::Object> ModuleLoader::ModuleLoaderImpl::ExportModule(v8::Local<v8::Object> moduleObject,
                                                                   const ModuleInitializer& moduleInitializer) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope scope(isolate);

    auto module = moduleObject->Get(v8_helpers::MakeV8String(isolate, "module"))->ToObject();
    auto exports = module->Get(v8_helpers::MakeV8String(isolate, "exports"))->ToObject();
    if (moduleInitializer != nullptr) {
        moduleInitializer(exports, moduleObject);
    }

    return scope.Escape(exports);
}