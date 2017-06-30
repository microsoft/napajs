#include "module-resolver.h"

#include <platform/platform.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

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
    ModuleInfo ResolveFromPath(const boost::filesystem::path& name, const boost::filesystem::path& path);

    /// <summary> It resolves a full module path from 'NODE_PATH'. </summary>
    /// <param name="name"> Module name or path. </param>
    /// <param name="path"> Base path. </param>
    /// <returns> Module resolution details. </returns>
    ModuleInfo ResolveFromEnv(const boost::filesystem::path& name, const boost::filesystem::path& path);

    /// <summary> It checks whether a module exists or not. </summary>
    /// <param name="name"> Module name. </param>
    /// <returns> True if a module exists at registry. </summary>
    bool IsCoreModule(const std::string& module);

    /// <summary> It loads module as a file. </summary>
    /// <param name="name"> Module name. </param>
    /// <param name="path"> Base path. </param>
    /// <param name="package"> Path to package.json. </param>
    /// <returns> Module resolution details. </returns>
    ModuleInfo LoadAsFile(const boost::filesystem::path& name, const boost::filesystem::path& path);

    /// <summary> It loads module as a directory. </summary>
    /// <param name="name"> Module path. </param>
    /// <param name="path"> Base path. </param>
    /// <returns> Module resolution details. </returns>
    ModuleInfo LoadAsDirectory(const boost::filesystem::path& name, const boost::filesystem::path& path);

    /// <summary> It loads module from node_modules directories. </summary>
    /// <param name="name"> Module name or path. </param>
    /// <param name="path"> Base path. </param>
    /// <returns> Module resolution details. </returns>
    ModuleInfo LoadNodeModules(const boost::filesystem::path& name, const boost::filesystem::path& path);

    /// <summary> It returns all possible node module paths. </summary>
    /// <param name="path"> Base path. </param>
    /// <returns> List of possible node_modules paths. </summary>
    std::vector<std::string> GetNodeModulesPaths(const boost::filesystem::path& path);

    /// <summary> It tries the extension candidates. </summary>
    /// <param name="path"> Possible module path. </param>
    /// <returns> Module resolution details. </returns>
    /// <remarks> "path" argument is changed. </remarks>
    ModuleInfo TryExtensions(const boost::filesystem::path& path);

    /// <summary> It checks whether a path is relative. </summary>
    /// <param name="path"> Path. </param>
    /// <returns> True if a path is relative. </summary>
    bool IsRelativePath(const boost::filesystem::path& path) const;

    /// <summary> It checks whether a path is absolute. </summary>
    /// <param name="path"> Path. </param>
    /// <returns> True if a path is absolute. </summary>
    bool IsAbsolutePath(const boost::filesystem::path& path) const;

    /// <summary> It combines two given paths. </summary>
    /// <param name="name"> Module name or path. </param>
    /// <param name="path"> Base path. </param>
    /// <returns> Combined path. </returns>
    boost::filesystem::path CombinePath(const boost::filesystem::path& name, const boost::filesystem::path& path);

    /// <summary> Registered modules. </summary>
    std::unordered_set<std::string> _coreModules;

    /// <summary> Paths in 'NODE_PATH' environment variable. </summary>
    std::vector<std::string> _nodePaths;
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
        boost::split(nodePaths, envPath, boost::is_any_of(platform::ENV_DELIMITER));
        for (auto& nodePath : nodePaths) {
            if (boost::filesystem::exists(nodePath)) {
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
    boost::filesystem::path basePath =
        (path == nullptr) ? boost::filesystem::current_path() : boost::filesystem::path(path);
    basePath = basePath.normalize().make_preferred();

    // Normalize module name.
    boost::filesystem::path moduleName(name);
    moduleName = moduleName.normalize().make_preferred();

    // Look up from the given path.
    RETURN_IF_NOT_EMPTY(ResolveFromPath(moduleName, basePath));

    // Look up NODE_PATH
    return ResolveFromEnv(moduleName, basePath);
}

bool ModuleResolver::ModuleResolverImpl::SetAsCoreModule(const char* name) {
    auto result = _coreModules.emplace(name);
    return result.second;
}

ModuleInfo ModuleResolver::ModuleResolverImpl::ResolveFromPath(const boost::filesystem::path& name,
                                                               const boost::filesystem::path& path) {
    // If name begins with './' or '/' or '../',
    if (IsRelativePath(name) || IsAbsolutePath(name)) {
        // Load as a file (path + name).
        RETURN_IF_NOT_EMPTY(LoadAsFile(name, path));

        // Load as a directory (path + name)
        RETURN_IF_NOT_EMPTY(LoadAsDirectory(name, path));
    }

    // Load node_modules.
    return LoadNodeModules(name, path);
}

ModuleInfo ModuleResolver::ModuleResolverImpl::ResolveFromEnv(const boost::filesystem::path& name,
                                                              const boost::filesystem::path& path) {
    for (const auto& nodePath : _nodePaths) {
        if (nodePath == path) {
            continue;
        }

        RETURN_IF_NOT_EMPTY(ResolveFromPath(name, nodePath.c_str()));
    }

    return ModuleInfo{ModuleType::NONE, std::string(), std::string()};
}

bool ModuleResolver::ModuleResolverImpl::IsCoreModule(const std::string& module) {
    return _coreModules.find(module) != _coreModules.end();
}

ModuleInfo ModuleResolver::ModuleResolverImpl::LoadAsFile(const boost::filesystem::path& name,
                                                          const boost::filesystem::path& path) {
    auto fullPath = CombinePath(name, path);

    if (boost::filesystem::is_regular_file(fullPath)) {
        ModuleType type = ModuleType::JAVASCRIPT;

        auto extension = boost::filesystem::extension(fullPath);
        if (extension == JSON_OBJECT_EXTENSION) {
            type = ModuleType::JSON;
        } else if (extension == NAPA_MODULE_EXTENSION) {
            type = ModuleType::NAPA;
        }

        return ModuleInfo{type, fullPath.string(), std::string()};
    }

    return TryExtensions(fullPath);
}

ModuleInfo ModuleResolver::ModuleResolverImpl::LoadAsDirectory(const boost::filesystem::path& name,
                                                               const boost::filesystem::path& path) {
    auto fullPath = CombinePath(name, path);

    auto packageJson = fullPath / "package.json";
    if (boost::filesystem::is_regular_file(packageJson)) {
        rapidjson::Document package;
        try {
            std::ifstream ifs(packageJson.string());
            rapidjson::IStreamWrapper isw(ifs);
            if (package.ParseStream(isw).HasParseError()) {
                throw std::runtime_error(rapidjson::GetParseError_En(package.GetParseError()));
            }

            boost::filesystem::path mainPath(package["main"].GetString());
            mainPath = mainPath.normalize().make_preferred();

            auto moduleInfo = LoadAsFile(mainPath, fullPath);
            if (moduleInfo.type != ModuleType::NONE) {
                moduleInfo.packageJsonPath = packageJson.string();
                return moduleInfo;
            }
        } catch (...) {}    // ignore exception and continue.
    }

    fullPath = fullPath / "index";
    return TryExtensions(fullPath);
}

ModuleInfo ModuleResolver::ModuleResolverImpl::LoadNodeModules(const boost::filesystem::path& name,
                                                               const boost::filesystem::path& path) {
    auto modulePaths = GetNodeModulesPaths(path);
    for (const auto& modulePath :modulePaths) {
        // Load as a file (path + name).
        RETURN_IF_NOT_EMPTY(LoadAsFile(name, modulePath));

        // Load as a directory (path + name)
        RETURN_IF_NOT_EMPTY(LoadAsDirectory(name, modulePath));
    }

    return ModuleInfo{ModuleType::NONE, std::string(), std::string()};
}

std::vector<std::string> ModuleResolver::ModuleResolverImpl::GetNodeModulesPaths(const boost::filesystem::path& path) {
    std::vector<std::string> subpaths;
    subpaths.reserve(256);

    for (boost::filesystem::path subpath(path); !subpath.empty(); subpath = subpath.parent_path()) {
        if (subpath.filename().string() == "node_modules") {
            continue;
        }

        auto modulePath = subpath / "node_modules";
        if (boost::filesystem::exists(modulePath)) {
            subpaths.emplace_back(modulePath.string());
        }
    }

    return subpaths;
}

ModuleInfo ModuleResolver::ModuleResolverImpl::TryExtensions(const boost::filesystem::path& path) {
    std::ostringstream oss;
    oss << path.string() << JAVASCRIPT_MODULE_EXTENSION;

    auto modulePath = boost::filesystem::path(oss.str());
    if (boost::filesystem::is_regular_file(modulePath)) {
        return ModuleInfo{ModuleType::JAVASCRIPT, modulePath.string(), std::string()};
    }

    modulePath.replace_extension(JSON_OBJECT_EXTENSION);
    if (boost::filesystem::is_regular_file(modulePath)) {
        return ModuleInfo{ModuleType::JSON, modulePath.string(), std::string()};
    }

    modulePath.replace_extension(NAPA_MODULE_EXTENSION);
    if (boost::filesystem::is_regular_file(modulePath)) {
        return ModuleInfo{ModuleType::NAPA, modulePath.string(), std::string()};
    }

    return ModuleInfo{ModuleType::NONE, std::string(), std::string()};
}

bool ModuleResolver::ModuleResolverImpl::IsRelativePath(const boost::filesystem::path& path) const {
    auto start = path.begin()->generic_string();
    return start == "." || start == "..";
}

bool ModuleResolver::ModuleResolverImpl::IsAbsolutePath(const boost::filesystem::path& path) const {
    auto start = path.begin()->generic_string();

    // Regarding boost document, path("/foo").is_absolute() returns false on windows.
    return path.is_absolute() || start == "/";
}

boost::filesystem::path ModuleResolver::ModuleResolverImpl::CombinePath(const boost::filesystem::path& name,
                                                                        const boost::filesystem::path& path) {
    auto combinedPath = (IsAbsolutePath(name)) ? name : path / name;
    return combinedPath.normalize().make_preferred();
}