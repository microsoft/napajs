// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <catch/catch.hpp>

#include <module/loader/module-resolver-cache.h>

using namespace napa;
using namespace napa::module;

namespace {
    ModuleResolverCache& GetModuleResolverCache() {
        static ModuleResolverCache cache;
        return cache;
    }
}

TEST_CASE("module resolver cache works correctly.", "[module-resolver-cache]") {

    SECTION("cache not hit") {
        ModuleResolverCache cache;
        auto result = cache.Lookup("a", "/home/napajs/test/");
        REQUIRE(result.type == ModuleType::NONE);
    }

    SECTION("cache hit") {
        ModuleResolverCache cache;
        cache.Insert("a", "/home/napajs/test/", ModuleInfo{ModuleType::JAVASCRIPT, "/home/napajs/test/a.js", std::string()});
        
        auto result = cache.Lookup("a", "/home/napajs/test/");
        REQUIRE(result.type == ModuleType::JAVASCRIPT);
        REQUIRE(result.fullPath == "/home/napajs/test/a.js");
    }
}
