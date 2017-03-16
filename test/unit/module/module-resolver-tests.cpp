#include "catch.hpp"

#include <module/module-resolver.h>
#include <napa/module/command-line.h>

#include <boost/filesystem.hpp>

#include <sstream>

using namespace napa::module;

namespace {

    ModuleResolver& GetModuleResolver() {
        // Set environment variables before module resolver is initialized.
        static bool setNodePath = []() {
            std::ostringstream oss;
            oss << (boost::filesystem::current_path() / "resolve-env").string()
                << command_line::ENV_DELIMITER
                << (boost::filesystem::current_path() / "child" / "node_modules" / "child").string();
            return command_line::SetEnv("NODE_PATH", oss.str().c_str());
        }();
        REQUIRE(setNodePath);

        static ModuleResolver resolver;
        return resolver;
    }

    void ResolveIt(const char* module, const char* expected) {
        REQUIRE(GetModuleResolver().Resolve(module) == expected);
    }

    void ResolveIt(const char* module, const boost::filesystem::path& expected) {
        REQUIRE(GetModuleResolver().Resolve(module) == expected.string());
    }

}   // Namespace of anonymous namespace.

TEST_CASE("resolve node modules correctly.", "[module-resolver]") {
    auto currentPath = boost::filesystem::current_path();

    SECTION("resolve built-in or core modules") {
        // Set build-in and core modules.
        GetModuleResolver().SetAsBuiltInOrCoreModule("console");
        GetModuleResolver().SetAsBuiltInOrCoreModule("fs");

        // 'console' built-in module.
        ResolveIt("console", "console");

        // 'fs' core module.
        ResolveIt("fs", "fs");

        // No 'built-in' module.
        ResolveIt("built-in", "");
    }

    SECTION("resolve as a file") {
        // Starts with "./" and a file exists.
        ResolveIt("./resolve-file", currentPath / "resolve-file");

        // Starts with "./" and a javascript file exists.
        ResolveIt("./resolve-file-js", currentPath / "resolve-file-js.js");

        // Starts with "./" and a json file exists.
        ResolveIt("./resolve-file-json", currentPath / "resolve-file-json.json");

        // Starts with "./" and a binary file exists.
        ResolveIt("./resolve-file-napa", currentPath / "resolve-file-napa.napa");

        // Starts with "./", but a file doesn't exist.
        ResolveIt("./resolve-file-non-existent", "");

        // Starts with "../" and a file exists.
        std::ostringstream oss;
        oss << "../" << currentPath.filename().string() << "/resolve-file";
        ResolveIt(oss.str().c_str(), currentPath / "resolve-file");

        // Starts with "/", but a file doesn't exist.
        ResolveIt("/resolve-file", "");

        // Use absolute path and a file exists.
        ResolveIt((currentPath / "resolve-file").string().c_str(), currentPath / "resolve-file");
    }

    SECTION("resolve as a directory") {
        // Starts with "./" and a directory with package.json exists.
        ResolveIt("./resolve-directory/resolver", currentPath / "resolve-directory" / "resolver" / "resolve-file");

        // Starts with "./" and a directory with index.js exists.
        ResolveIt("./resolve-directory/resolver-js", currentPath / "resolve-directory" / "resolver-js" / "index.js");

        // Starts with "./" and a directory with index.json exists.
        ResolveIt("./resolve-directory/resolver-json", currentPath / "resolve-directory" / "resolver-json" / "index.json");

        // Starts with "./" and a directory with index.napa exists.
        ResolveIt("./resolve-directory/resolver-napa", currentPath / "resolve-directory" / "resolver-napa" / "index.napa");

        // Non existent directory.
        ResolveIt("./resolve-directory/resolver-non-existent", "");

        // Starts with "../" and a directory exists.
        std::ostringstream oss;
        oss << "../" << currentPath.filename().string() << "/resolve-directory/resolver";
        ResolveIt(oss.str().c_str(), currentPath / "resolve-directory" / "resolver" / "resolve-file");

        // Starts with "/", but a file doesn't exist.
        ResolveIt("/resolve-directory/resolver", "");

        // Use absolute path and a file exists.
        ResolveIt((currentPath / "resolve-directory" / "resolver").string().c_str(), currentPath / "resolve-directory" / "resolver" / "resolve-file");
    }

    SECTION("resolve from node_modules") {
        // A file exists.
        ResolveIt("resolve-nm-file", currentPath / "node_modules" / "resolve-nm-file");

        // A javascript file exists.
        ResolveIt("resolve-nm-file-js", currentPath / "node_modules" / "resolve-nm-file-js.js");

        // A json file exists.
        ResolveIt("resolve-nm-file-json", currentPath / "node_modules" / "resolve-nm-file-json.json");

        // A binary file exists.
        ResolveIt("resolve-nm-file-napa", currentPath / "node_modules" / "resolve-nm-file-napa.napa");

        // A directory with package.json exists.
        ResolveIt("resolver-nm", currentPath / "node_modules" / "resolver-nm" / "resolve-file");

        // A directory with index.js exists.
        ResolveIt("resolver-nm-js", currentPath / "node_modules" / "resolver-nm-js" / "index.js");

        // A directory with index.json exists.
        ResolveIt("resolver-nm-json", currentPath / "node_modules" / "resolver-nm-json" / "index.json");

        // A directory with index.napa exists.
        ResolveIt("resolver-nm-napa", currentPath / "node_modules" / "resolver-nm-napa" / "index.napa");
    }

    SECTION("resolve from NODE_PATH") {
        // Starts with "./" and a file exists.
        ResolveIt("./resolve-env-file", currentPath / "resolve-env" / "resolve-env-file");

        // Starts with "./" and a javascript file exists.
        ResolveIt("./resolve-env-file-js", currentPath / "resolve-env" / "resolve-env-file-js.js");

        // Starts with "./" and a json file exists.
        ResolveIt("./resolve-env-file-json", currentPath / "resolve-env" / "resolve-env-file-json.json");

        // Starts with "./" and a binary file exists.
        ResolveIt("./resolve-env-file-napa", currentPath / "resolve-env" / "resolve-env-file-napa.napa");

        // Starts with "../" and a file exists.
        ResolveIt("../resolve-env/resolve-env-file", currentPath / "resolve-env" / "resolve-env-file");

        // Starts with "./" and a directory with package.json exists.
        ResolveIt("./resolver-env", currentPath / "resolve-env" / "resolver-env" / "resolve-file");

        // Starts with "./" and a directory with index.js exists.
        ResolveIt("./resolver-env-js", currentPath /"resolve-env" / "resolver-env-js" / "index.js");

        // Starts with "./" and a directory with index.json exists.
        ResolveIt("./resolver-env-json", currentPath / "resolve-env" / "resolver-env-json" / "index.json");

        // Starts with "./" and a directory with index.napa exists.
        ResolveIt("./resolver-env-napa", currentPath / "resolve-env" / "resolver-env-napa" / "index.napa");

        // Starts with "../" and a directory exists.
        ResolveIt("../resolve-env/resolver-env", currentPath / "resolve-env" / "resolver-env" / "resolve-file");
    }

    SECTION("resolve from parent node_modules") {
        // A file exists.
        ResolveIt("resolve-cc-file", currentPath / "child" / "node_modules" / "resolve-cc-file");

        // A javascript file exists.
        ResolveIt("resolve-cc-file-js", currentPath / "child" / "node_modules" / "resolve-cc-file-js.js");

        // A json file exists.
        ResolveIt("resolve-cc-file-json", currentPath / "child" / "node_modules" / "resolve-cc-file-json.json");

        // A binary file exists.
        ResolveIt("resolve-cc-file-napa", currentPath / "child" / "node_modules" / "resolve-cc-file-napa.napa");

        // A directory with package.json exists.
        ResolveIt("resolver-cc", currentPath / "child" / "node_modules" / "resolver-cc" / "resolve-file");

        // A directory with index.js exists.
        ResolveIt("resolver-cc-js", currentPath / "child" / "node_modules" / "resolver-cc-js" / "index.js");

        // A directory with index.json exists.
        ResolveIt("resolver-cc-json", currentPath / "child" / "node_modules" / "resolver-cc-json" / "index.json");

        // A directory with index.napa exists.
        ResolveIt("resolver-cc-napa", currentPath / "child" / "node_modules" / "resolver-cc-napa" / "index.napa");
    }
}
