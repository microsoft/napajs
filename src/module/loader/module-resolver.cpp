// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "module-resolver.h"
#include "module-resolver-cache.h"

#include <platform/filesystem.h>
#include <platform/os.h>
#include <platform/process.h>

#include <utils/string.h>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

#include <fstream>
#include <sstream>
#include <unordered_set>

using namespace napa;
using namespace napa::module;

namespace {

    const std::string NAPA_MODULE_EXTENSION = ".napa";
    const std::string JAVASCRIPT_MODULE_EXTENSION = ".js";
    const std::string JSON_OBJECT_EXTENSION = ".json";

}   // End of anonymous namespace.

class ModuleResolver::ModuleResolverImpl {
public:

    /// <summary> Constructor. </summary>
    ModuleResolverImpl();

    /// <summary> It resolves a full module path from a given argument of require(). </summary>
    /// <param name="name"> Module name or path. </param>
    /// <param name="path"> Current context path. </param>
    /// <returns> Module resolution details. </returns>
    /// <remarks> It also searches NODE_PATH directories. </remarks>
    ModuleInfo Resolve(const char* name, const char* path);

    /// <summary> It registers core modules, so they are resolved first. </summary>
    /// <param name="name"> Module name. </param>
    /// <returns>
    /// True if it successfully adds a module.
    /// If it fails or there is a duplication, return false.
    /// </returns>
    bool SetAsCoreModule(const char* name);

private:

    /// <summary> It resolves a full module path from a given argument of require(). </summary>
    /// <param name="name"> Module name or path. </param>
    /// <param name="path"> Base path. </param>
    /// <returns> Module resolution details. </returns>
    ModuleInfo ResolveFromPath(const filesystem::Path& name, const filesystem::Path& path);

    /// <summary> It resolves a full module path from 'NODE_PATH'. </summary>
    /// <param name="name"> Module name or path. </param>
    /// <param name="path"> Base path. </param>
    /// <returns> Module resolution details. </returns>
    ModuleInfo ResolveFromEnv(const filesystem::Path& name, const filesystem::Path& path);

    /// <summary> It checks whether a module exists or not. </summary>
    /// <param name="name"> Module name. </param>
    /// <returns> True if a module exists at registry. </summary>
    bool IsCoreModule(const std::string& module);

    /// <summary> It loads module as a file. </summary>
    /// <param name="name"> Module name. </param>
    /// <param name="path"> Base path. </param>
    /// <param name="package"> Path to package.json. </param>
    /// <returns> Module resolution details. </returns>
    ModuleInfo LoadAsFile(const filesystem::Path& name, const filesystem::Path& path);

    /// <summary> It loads module as a directory. </summary>
    /// <param name="name"> Module path. </param>
    /// <param name="path"> Base path. </param>
    /// <returns> Module resolution details. </returns>
    ModuleInfo LoadAsDirectory(const filesystem::Path& name, const filesystem::Path& path);

    /// <summary> It loads module from node_modules directories. </summary>
    /// <param name="name"> Module name or path. </param>
    /// <param name="path"> Base path. </param>
    /// <returns> Module resolution details. </returns>
    ModuleInfo LoadNodeModules(const filesystem::Path& name, const filesystem::Path& path);

    /// <summary> It returns all possible node module paths. </summary>
    /// <param name="path"> Base path. </param>
    /// <returns> List of possible node_modules paths. </summary>
    std::vector<std::string> GetNodeModulesPaths(const filesystem::Path& path);

    /// <summary> It tries the extension candidates. </summary>
    /// <param name="path"> Possible module path. </param>
    /// <returns> Module resolution details. </returns>
    /// <remarks> "path" argument is changed. </remarks>
    ModuleInfo TryExtensions(const filesystem::Path& path);

    /// <summary> It checks whether a path is relative. </summary>
    /// <param name="path"> Path. </param>
    /// <returns> True if a path is relative. </summary>
    bool IsExplicitRelativePath(const filesystem::Path& path) const;

    /// <summary> Registered modules. </summary>
    std::unordered_set<std::string> _coreModules;

    /// <summary> Paths in 'NODE_PATH' environment variable. </summary>
    std::vector<std::string> _nodePaths;

    /// <summary> Module info cache for all loaded modules. </summary>
    ModuleResolverCache _cache;
};

ModuleResolver::ModuleResolver() : _impl(std::make_unique<ModuleResolver::ModuleResolverImpl>()) {}

ModuleResolver::~ModuleResolver() = default;

ModuleInfo ModuleResolver::Resolve(const char* name, const char* path) {
    return _impl->Resolve(name, path);
}

bool ModuleResolver::SetAsCoreModule(const char* name) {
    return _impl->SetAsCoreModule(name);
}

ModuleResolver::ModuleResolverImpl::ModuleResolverImpl() {
    auto envPath = platform::GetEnv("NODE_PATH");
    if (!envPath.empty()) {
        std::vector<std::string> nodePaths;
        utils::string::Split(envPath, nodePaths, std::string(platform::ENV_DELIMITER));

        for (auto& nodePath : nodePaths) {
            if (filesystem::IsDirectory(nodePath)) {
                _nodePaths.emplace_back(std::move(nodePath));
            }
        }
    }
}

#define RETURN_IF_NOT_EMPTY(run)                    \
    do {                                            \
        auto result = run;                          \
        if (result.type != ModuleType::NONE) {      \
            return result;                          \
        }                                           \
    } while (false);

ModuleInfo ModuleResolver::ModuleResolverImpl::Resolve(const char* name, const char* path) {
    // If name is a core modules, return it.
    if (IsCoreModule(name)) {
        return ModuleInfo{ModuleType::CORE, std::string(name), std::string()};
    }

    // Normalize module context path.
    filesystem::Path basePath =
        (path == nullptr) ? filesystem::CurrentDirectory() : filesystem::Path(path);

    // Lookup for module info cache.
    RETURN_IF_NOT_EMPTY(_cache.Lookup(name, basePath.c_str()));
    
        // Look up from the given path.
    ModuleInfo moduleInfo = ResolveFromPath(name, basePath);
    if (moduleInfo.type != ModuleType::NONE) {
        _cache.Insert(name, basePath.c_str(), moduleInfo);
        return moduleInfo;
    }

    // Look up NODE_PATH
    moduleInfo = ResolveFromEnv(name, basePath);
    if (moduleInfo.type != ModuleType::NONE) {
        _cache.Insert(name, basePath.c_str(), moduleInfo);
    }
    return moduleInfo;
}

bool ModuleResolver::ModuleResolverImpl::SetAsCoreModule(const char* name) {
    auto result = _coreModules.emplace(name);
    return result.second;
}

ModuleInfo ModuleResolver::ModuleResolverImpl::ResolveFromPath(const filesystem::Path& name,
                                                               const filesystem::Path& path) {
    // If name begins with './' or '/' or '../',
    if (IsExplicitRelativePath(name) || name.IsAbsolute()) {
        // Load as a file (path + name).
        RETURN_IF_NOT_EMPTY(LoadAsFile(name, path));

        // Load as a directory (path + name)
        RETURN_IF_NOT_EMPTY(LoadAsDirectory(name, path));
    }

    // Load node_modules.
    return LoadNodeModules(name, path);
}

ModuleInfo ModuleResolver::ModuleResolverImpl::ResolveFromEnv(const filesystem::Path& name,
                                                              const filesystem::Path& path) {
    for (const auto& nodePath : _nodePaths) {
        if (nodePath == path.String()) {
            continue;
        }

        RETURN_IF_NOT_EMPTY(ResolveFromPath(name, nodePath.c_str()));
    }

    return ModuleInfo{ModuleType::NONE, std::string(), std::string()};
}

bool ModuleResolver::ModuleResolverImpl::IsCoreModule(const std::string& module) {
    return _coreModules.find(module) != _coreModules.end();
}

ModuleInfo ModuleResolver::ModuleResolverImpl::LoadAsFile(const filesystem::Path& name,
                                                          const filesystem::Path& path) {
    auto fullPath = (path / name).Normalize();

    if (filesystem::IsRegularFile(fullPath)) {
        ModuleType type = ModuleType::JAVASCRIPT;

        auto extension = fullPath.Extension().String();
        if (extension == JSON_OBJECT_EXTENSION) {
            type = ModuleType::JSON;
        } else if (extension == NAPA_MODULE_EXTENSION) {
            type = ModuleType::NAPA;
        }

        return ModuleInfo{type, fullPath.String(), std::string()};
    }

    return TryExtensions(fullPath);
}

ModuleInfo ModuleResolver::ModuleResolverImpl::LoadAsDirectory(const filesystem::Path& name,
                                                               const filesystem::Path& path) {
    auto fullPath = (path / name).Normalize();

    auto packageJson = fullPath / "package.json";
    if (filesystem::IsRegularFile(packageJson)) {
        rapidjson::Document package;
        try {
            std::ifstream ifs(packageJson.String());
            rapidjson::IStreamWrapper isw(ifs);
            if (package.ParseStream(isw).HasParseError()) {
                throw std::runtime_error(rapidjson::GetParseError_En(package.GetParseError()));
            }

            if (package.HasMember("main")) {
                filesystem::Path mainPath(package["main"].GetString());
                mainPath.Normalize();

                auto moduleInfo = LoadAsFile(mainPath, fullPath);
                if (moduleInfo.type != ModuleType::NONE) {
                    moduleInfo.packageJsonPath = packageJson.String();
                    return moduleInfo;
                }
            }
        } catch (...) {}    // ignore exception and continue.
    }

    fullPath = fullPath / "index";
    return TryExtensions(fullPath);
}

ModuleInfo ModuleResolver::ModuleResolverImpl::LoadNodeModules(const filesystem::Path& name,
                                                               const filesystem::Path& path) {
    auto modulePaths = GetNodeModulesPaths(path);
    for (const auto& modulePath :modulePaths) {
        // Load as a file (path + name).
        RETURN_IF_NOT_EMPTY(LoadAsFile(name, modulePath));

        // Load as a directory (path + name)
        RETURN_IF_NOT_EMPTY(LoadAsDirectory(name, modulePath));
    }

    return ModuleInfo{ModuleType::NONE, std::string(), std::string()};
}

std::vector<std::string> ModuleResolver::ModuleResolverImpl::GetNodeModulesPaths(const filesystem::Path& path) {
    std::vector<std::string> subpaths;
    subpaths.reserve(256);

    for (filesystem::Path subpath(path); !subpath.IsEmpty(); subpath = subpath.Parent().Normalize()) {
        if (subpath.Filename().String() == "node_modules") {
            continue;
        }

        auto modulePath = subpath / "node_modules";
        if (filesystem::IsDirectory(modulePath)) {
            subpaths.emplace_back(modulePath.String());
        }
    }

    return subpaths;
}

ModuleInfo ModuleResolver::ModuleResolverImpl::TryExtensions(const filesystem::Path& path) {
    std::ostringstream oss;
    oss << path.String() << JAVASCRIPT_MODULE_EXTENSION;

    auto modulePath = filesystem::Path(oss.str());
    if (filesystem::IsRegularFile(modulePath)) {
        return ModuleInfo{ModuleType::JAVASCRIPT, modulePath.String(), std::string()};
    }

    modulePath.ReplaceExtension(JSON_OBJECT_EXTENSION);
    if (filesystem::IsRegularFile(modulePath)) {
        return ModuleInfo{ModuleType::JSON, modulePath.String(), std::string()};
    }

    modulePath.ReplaceExtension(NAPA_MODULE_EXTENSION);
    if (filesystem::IsRegularFile(modulePath)) {
        return ModuleInfo{ModuleType::NAPA, modulePath.String(), std::string()};
    }

    return ModuleInfo{ModuleType::NONE, std::string(), std::string()};
}

bool ModuleResolver::ModuleResolverImpl::IsExplicitRelativePath(const filesystem::Path& path) const {
    // Start with "." or ".."
    return !path.IsEmpty() && path.String()[0] == '.';
}
