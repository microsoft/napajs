#include "module-resolver.h"

#include <napa/module/command-line.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>

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
    /// <returns> Full module path. </returns>
    /// <remarks> It also searches NODE_PATH directories. </remarks>
    std::string Resolve(const char* name, const char* path);

    /// <summary> It registers built-in or core modules, so they are resolved first. </summary>
    /// <param name="name"> Module name. </param>
    /// <returns>
    /// True if it successfully adds a module.
    /// If it fails or there is a duplication, return false.
    /// </returns>
    bool SetAsBuiltInOrCoreModule(const char* name);

private:

    /// <summary> It resolves a full module path from a given argument of require(). </summary>
    /// <param name="name"> Module name or path. </param>
    /// <param name="path"> Base path. </param>
    /// <returns> Full module path. </returns>
    std::string ResolveFromPath(const boost::filesystem::path& name, const boost::filesystem::path& path);

    /// <summary> It resolves a full module path from 'NODE_PATH'. </summary>
    /// <param name="name"> Module name or path. </param>
    /// <param name="path"> Base path. </param>
    /// <returns> Full module path. </returns>
    std::string ResolveFromEnv(const boost::filesystem::path& name, const boost::filesystem::path& path);

    /// <summary> It checks whether a module exists or not. </summary>
    /// <param name="name"> Module name. </param>
    /// <returns> True if a module exists at registry. </summary>
    bool IsBuiltInOrCoreModule(const std::string& module);

    /// <summary> It loads module as a file. </summary>
    /// <param name="name"> Module name. </param>
    /// <param name="path"> Base path. </param>
    /// <returns> Resolved module path if it succeeds. Empty string, otherwise. </returns>
    std::string LoadAsFile(const boost::filesystem::path& name, const boost::filesystem::path& path);

    /// <summary> It loads module as a directory. </summary>
    /// <param name="name"> Module path. </param>
    /// <param name="path"> Base path. </param>
    /// <returns> Resolved module path if it succeeds. Empty string, otherwise. </returns>
    std::string LoadAsDirectory(const boost::filesystem::path& name, const boost::filesystem::path& path);

    /// <summary> It loads module from node_modules directories. </summary>
    /// <param name="name"> Module name or path. </param>
    /// <param name="path"> Base path. </param>
    /// <returns> Resolved module path if it succeeds. Empty string, otherwise. </returns>
    std::string LoadNodeModules(const boost::filesystem::path& name, const boost::filesystem::path& path);

    /// <summary> It returns all possible node module paths. </summary>
    /// <param name="path"> Base path. </param>
    /// <returns> List of possible node_modules paths. </summary>
    std::vector<std::string> GetNodeModulesPaths(const boost::filesystem::path& path);

    /// <summary> It tries the extension candidates. </summary>
    /// <param name="path"> Possible module path. </param>
    /// <returns> Resolved module path if it succeeds. Empty string, otherwise. </returns>
    /// <remarks> "path" argument is changed. </remarks>
    std::string TryExtensions(boost::filesystem::path& path);

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
    std::unordered_set<std::string> _builtInOrCoreModules;

    /// <summary> Paths in 'NODE_PATH' environment variable. </summary>
    std::vector<std::string> _nodePaths;
};

ModuleResolver::ModuleResolver() : _impl(std::make_unique<ModuleResolver::ModuleResolverImpl>()) {}

ModuleResolver::~ModuleResolver() = default;

std::string ModuleResolver::Resolve(const char* name, const char* path) {
    return _impl->Resolve(name, path);
}

bool ModuleResolver::SetAsBuiltInOrCoreModule(const char* name) {
    return _impl->SetAsBuiltInOrCoreModule(name);
}

ModuleResolver::ModuleResolverImpl::ModuleResolverImpl() {
    auto envPath = command_line::GetEnv("NODE_PATH");
    if (!envPath.empty()) {
        std::vector<std::string> nodePaths;
        boost::split(nodePaths, envPath, boost::is_any_of(command_line::ENV_DELIMITER));
        for (auto& nodePath : nodePaths) {
            if (boost::filesystem::exists(nodePath)) {
                _nodePaths.emplace_back(std::move(nodePath));
            }
        }
    }
}

#define RETURN_IF_NOT_EMPTY(run)    \
    do {                            \
        auto result = run;          \
        if (!result.empty()) {      \
            return result;          \
        }                           \
    } while (false);

std::string ModuleResolver::ModuleResolverImpl::Resolve(const char* name, const char* path) {
    // If name is a built-in or core modules, return it.
    if (IsBuiltInOrCoreModule(name)) {
        return std::string(name);
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

bool ModuleResolver::ModuleResolverImpl::SetAsBuiltInOrCoreModule(const char* name) {
    auto result = _builtInOrCoreModules.emplace(name);
    return result.second;
}

std::string ModuleResolver::ModuleResolverImpl::ResolveFromPath(const boost::filesystem::path& name,
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

std::string ModuleResolver::ModuleResolverImpl::ResolveFromEnv(const boost::filesystem::path& name,
                                                               const boost::filesystem::path& path) {
    for (const auto& nodePath : _nodePaths) {
        if (nodePath == path) {
            continue;
        }

        RETURN_IF_NOT_EMPTY(ResolveFromPath(name, nodePath.c_str()));
    }

    return std::string();
}

bool ModuleResolver::ModuleResolverImpl::IsBuiltInOrCoreModule(const std::string& module) {
    return _builtInOrCoreModules.find(module) != _builtInOrCoreModules.end();
}

std::string ModuleResolver::ModuleResolverImpl::LoadAsFile(const boost::filesystem::path& name,
                                                           const boost::filesystem::path& path) {
    auto fullPath = CombinePath(name, path);

    if (boost::filesystem::is_regular_file(fullPath)) {
        return fullPath.string();
    }

    return TryExtensions(fullPath);
}

std::string ModuleResolver::ModuleResolverImpl::LoadAsDirectory(const boost::filesystem::path& name,
                                                                const boost::filesystem::path& path) {
    auto fullPath = CombinePath(name, path);

    auto packageJson = fullPath / "package.json";
    if (boost::filesystem::is_regular_file(packageJson)) {
        boost::property_tree::ptree package;
        try {
            boost::property_tree::json_parser::read_json(packageJson.string(), package);

            boost::filesystem::path mainPath(package.get<std::string>("main"));
            mainPath = mainPath.normalize().make_preferred();

            RETURN_IF_NOT_EMPTY(LoadAsFile(mainPath, fullPath));
        } catch (...) {}    // ignore exception and continue.
    }

    fullPath = fullPath / "index";
    return TryExtensions(fullPath);
}

std::string ModuleResolver::ModuleResolverImpl::LoadNodeModules(const boost::filesystem::path& name,
                                                                const boost::filesystem::path& path) {
    auto modulePaths = GetNodeModulesPaths(path);
    for (const auto& modulePath :modulePaths) {
        // Load as a file (path + name).
        RETURN_IF_NOT_EMPTY(LoadAsFile(name, modulePath));

        // Load as a directory (path + name)
        RETURN_IF_NOT_EMPTY(LoadAsDirectory(name, modulePath));
    }

    return std::string();
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

std::string ModuleResolver::ModuleResolverImpl::TryExtensions(boost::filesystem::path& path) {
    path.replace_extension(JAVASCRIPT_MODULE_EXTENSION);
    if (boost::filesystem::is_regular_file(path)) {
        return path.string();
    }

    path.replace_extension(JSON_OBJECT_EXTENSION);
    if (boost::filesystem::is_regular_file(path)) {
        return path.string();
    }

    path.replace_extension(NAPA_MODULE_EXTENSION);
    if (boost::filesystem::is_regular_file(path)) {
        return path.string();
    }

    return std::string();
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
