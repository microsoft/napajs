#include <catch/catch.hpp>

#include <module/loader/module-resolver.h>
#include <platform/filesystem.h>
#include <platform/platform.h>

#include <sstream>

using namespace napa;
using namespace napa::module;


namespace {

    ModuleResolver& GetModuleResolver() {
        // Set environment variables before module resolver is initialized.
        static bool setNodePath = []() {
            std::ostringstream oss;
            oss << (filesystem::CurrentDirectory() / "resolve-env").String()
                << platform::ENV_DELIMITER
                << (filesystem::CurrentDirectory() / "child" / "node_modules" / "child").String();
            return platform::SetEnv("NODE_PATH", oss.str().c_str());
        }();
        REQUIRE(setNodePath);

        static ModuleResolver resolver;
        return resolver;
    }

    void ResolveIt(const char* module,
                   const char* expected,
                   ModuleType type) {
        auto detail = GetModuleResolver().Resolve(module);

        REQUIRE(detail.fullPath == expected);
        REQUIRE(detail.type == type);
    }

    void ResolveIt(const char* module,
                   const filesystem::Path& expected,
                   ModuleType type) {
        auto detail = GetModuleResolver().Resolve(module);

        REQUIRE(detail.fullPath == expected);
        REQUIRE(detail.type == type);
    }

    void ResolveIt(const char* module,
                   const filesystem::Path& expected,
                   ModuleType type,
                   const filesystem::Path& package) {
        auto detail = GetModuleResolver().Resolve(module);

        REQUIRE(detail.fullPath == expected);
        REQUIRE(detail.type == type);
        REQUIRE(detail.packageJsonPath == package.String());
    }

}   // Namespace of anonymous namespace.

TEST_CASE("resolve node modules correctly.", "[module-resolver]") {
    auto currentPath = filesystem::CurrentDirectory();

    SECTION("resolve built-in or core modules") {
        // Set build-in and core modules.
        GetModuleResolver().SetAsCoreModule("console");
        GetModuleResolver().SetAsCoreModule("fs");

        // 'console' built-in module.
        ResolveIt("console", "console", ModuleType::CORE);

        // 'fs' core module.
        ResolveIt("fs", "fs", ModuleType::CORE);

        // No 'built-in' module.
        ResolveIt("built-in", "", ModuleType::NONE);
    }

    SECTION("resolve as a file") {
        // Starts with "./" and a file exists.
        ResolveIt("./resolve-file", currentPath / "resolve-file", ModuleType::JAVASCRIPT);

        // Starts with "./" and a javascript file exists.
        ResolveIt("./resolve-file-js", currentPath / "resolve-file-js.js", ModuleType::JAVASCRIPT);

        // Starts with "./" and a json file exists.
        ResolveIt("./resolve-file-json", currentPath / "resolve-file-json.json", ModuleType::JSON);

        // Starts with "./" and a binary file exists.
        ResolveIt("./resolve-file-napa", currentPath / "resolve-file-napa.napa", ModuleType::NAPA);

        // Starts with "./" and a javascript file exists.
        ResolveIt("./resolve-file-js.js", currentPath / "resolve-file-js.js", ModuleType::JAVASCRIPT);

        // Starts with "./" and a json file exists.
        ResolveIt("./resolve-file-js.json", currentPath / "resolve-file-js.json", ModuleType::JSON);

        // Starts with "./" and a binary file exists.
        ResolveIt("./resolve-file-js.napa", currentPath / "resolve-file-js.napa", ModuleType::NAPA);

        // Starts with "./", but a file doesn't exist.
        ResolveIt("./resolve-file-non-existent", "", ModuleType::NONE);

        // Start with "./", but a file doesn't exist and it doesn't resolve to ".js" file.
        ResolveIt("./resolve-file-no.json", "", ModuleType::NONE);

        // Start with "./", but a file doesn't exist and it doesn't resolve to ".js" file.
        ResolveIt("./resolve-file-no.napa", "", ModuleType::NONE);

        // Starts with "../" and a file exists.
        std::ostringstream oss;
        oss << "../" << currentPath.Filename().String() << "/resolve-file";
        ResolveIt(oss.str().c_str(), currentPath / "resolve-file", ModuleType::JAVASCRIPT);

        // Starts with "/", but a file doesn't exist.
        ResolveIt("/resolve-file", "", ModuleType::NONE);

        // Use absolute path and a file exists.
        ResolveIt((currentPath / "resolve-file").c_str(),
                  currentPath / "resolve-file",
                  ModuleType::JAVASCRIPT);
    }

    SECTION("resolve as a directory") {
        // Starts with "./" and a directory with package.json exists.
        ResolveIt("./resolve-directory/resolver",
                  currentPath / "resolve-directory" / "resolver" / "resolve-file",
                  ModuleType::JAVASCRIPT,
                  currentPath / "resolve-directory" / "resolver" / "package.json");

        // Starts with "./" and a directory with index.js exists.
        ResolveIt("./resolve-directory/resolver-js",
                  currentPath / "resolve-directory" / "resolver-js" / "index.js",
                  ModuleType::JAVASCRIPT);

        // Starts with "./" and a directory with index.json exists.
        ResolveIt("./resolve-directory/resolver-json",
                  currentPath / "resolve-directory" / "resolver-json" / "index.json",
                  ModuleType::JSON);

        // Starts with "./" and a directory with index.napa exists.
        ResolveIt("./resolve-directory/resolver-napa",
                  currentPath / "resolve-directory" / "resolver-napa" / "index.napa",
                  ModuleType::NAPA);

        // Non existent directory.
        ResolveIt("./resolve-directory/resolver-non-existent", "", ModuleType::NONE);

        // Starts with "../" and a directory exists.
        std::ostringstream oss;
        oss << "../" << currentPath.Filename().String() << "/resolve-directory/resolver";
        ResolveIt(oss.str().c_str(),
                  currentPath / "resolve-directory" / "resolver" / "resolve-file",
                  ModuleType::JAVASCRIPT,
                  currentPath / "resolve-directory" / "resolver" / "package.json");

        // Starts with "/", but a file doesn't exist.
        ResolveIt("/resolve-directory/resolver", "", ModuleType::NONE);

        // Use absolute path and a file exists.
        ResolveIt((currentPath / "resolve-directory" / "resolver").c_str(),
                  currentPath / "resolve-directory" / "resolver" / "resolve-file",
                  ModuleType::JAVASCRIPT,
                  currentPath / "resolve-directory" / "resolver" / "package.json");
    }

    SECTION("resolve from node_modules") {
        // A file exists.
        ResolveIt("resolve-nm-file", currentPath / "node_modules" / "resolve-nm-file", ModuleType::JAVASCRIPT);

        // A javascript file exists.
        ResolveIt("resolve-nm-file-js", currentPath / "node_modules" / "resolve-nm-file-js.js", ModuleType::JAVASCRIPT);

        // A json file exists.
        ResolveIt("resolve-nm-file-json", currentPath / "node_modules" / "resolve-nm-file-json.json", ModuleType::JSON);

        // A binary file exists.
        ResolveIt("resolve-nm-file-napa", currentPath / "node_modules" / "resolve-nm-file-napa.napa", ModuleType::NAPA);

        // A directory with package.json exists.
        ResolveIt("resolver-nm",
                  currentPath / "node_modules" / "resolver-nm" / "resolve-file",
                  ModuleType::JAVASCRIPT,
                  currentPath / "node_modules" / "resolver-nm" / "package.json");

        // A directory with index.js exists.
        ResolveIt("resolver-nm-js", currentPath / "node_modules" / "resolver-nm-js" / "index.js", ModuleType::JAVASCRIPT);

        // A directory with index.json exists.
        ResolveIt("resolver-nm-json", currentPath / "node_modules" / "resolver-nm-json" / "index.json", ModuleType::JSON);

        // A directory with index.napa exists.
        ResolveIt("resolver-nm-napa", currentPath / "node_modules" / "resolver-nm-napa" / "index.napa", ModuleType::NAPA);
    }

    SECTION("resolve from NODE_PATH") {
        // Starts with "./" and a file exists.
        ResolveIt("./resolve-env-file", currentPath / "resolve-env" / "resolve-env-file", ModuleType::JAVASCRIPT);

        // Starts with "./" and a javascript file exists.
        ResolveIt("./resolve-env-file-js", currentPath / "resolve-env" / "resolve-env-file-js.js", ModuleType::JAVASCRIPT);

        // Starts with "./" and a json file exists.
        ResolveIt("./resolve-env-file-json", currentPath / "resolve-env" / "resolve-env-file-json.json", ModuleType::JSON);

        // Starts with "./" and a binary file exists.
        ResolveIt("./resolve-env-file-napa", currentPath / "resolve-env" / "resolve-env-file-napa.napa", ModuleType::NAPA);

        // Starts with "../" and a file exists.
        ResolveIt("../resolve-env/resolve-env-file", currentPath / "resolve-env" / "resolve-env-file", ModuleType::JAVASCRIPT);

        // Starts with "./" and a directory with package.json exists.
        ResolveIt("./resolver-env",
                  currentPath / "resolve-env" / "resolver-env" / "resolve-file",
                  ModuleType::JAVASCRIPT,
                  currentPath / "resolve-env" / "resolver-env" / "package.json");

        // Starts with "./" and a directory with index.js exists.
        ResolveIt("./resolver-env-js", currentPath /"resolve-env" / "resolver-env-js" / "index.js", ModuleType::JAVASCRIPT);

        // Starts with "./" and a directory with index.json exists.
        ResolveIt("./resolver-env-json", currentPath / "resolve-env" / "resolver-env-json" / "index.json", ModuleType::JSON);

        // Starts with "./" and a directory with index.napa exists.
        ResolveIt("./resolver-env-napa", currentPath / "resolve-env" / "resolver-env-napa" / "index.napa", ModuleType::NAPA);

        // Starts with "../" and a directory exists.
        ResolveIt("../resolve-env/resolver-env",
                  currentPath / "resolve-env" / "resolver-env" / "resolve-file",
                  ModuleType::JAVASCRIPT,
                  currentPath / "resolve-env" / "resolver-env" / "package.json");
    }

    SECTION("resolve from parent node_modules") {
        // A file exists.
        ResolveIt("resolve-cc-file", currentPath / "child" / "node_modules" / "resolve-cc-file", ModuleType::JAVASCRIPT);

        // A javascript file exists.
        ResolveIt("resolve-cc-file-js", currentPath / "child" / "node_modules" / "resolve-cc-file-js.js", ModuleType::JAVASCRIPT);

        // A json file exists.
        ResolveIt("resolve-cc-file-json", currentPath / "child" / "node_modules" / "resolve-cc-file-json.json", ModuleType::JSON);

        // A binary file exists.
        ResolveIt("resolve-cc-file-napa", currentPath / "child" / "node_modules" / "resolve-cc-file-napa.napa", ModuleType::NAPA);

        // A directory with package.json exists.
        ResolveIt("resolver-cc",
                  currentPath / "child" / "node_modules" / "resolver-cc" / "resolve-file",
                  ModuleType::JAVASCRIPT,
                  currentPath / "child" / "node_modules" / "resolver-cc" / "package.json");

        // A directory with index.js exists.
        ResolveIt("resolver-cc-js", currentPath / "child" / "node_modules" / "resolver-cc-js" / "index.js", ModuleType::JAVASCRIPT);

        // A directory with index.json exists.
        ResolveIt("resolver-cc-json", currentPath / "child" / "node_modules" / "resolver-cc-json" / "index.json", ModuleType::JSON);

        // A directory with index.napa exists.
        ResolveIt("resolver-cc-napa", currentPath / "child" / "node_modules" / "resolver-cc-napa" / "index.napa", ModuleType::NAPA);
    }
}
