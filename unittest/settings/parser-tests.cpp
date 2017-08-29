// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <catch/catch.hpp>

#include <settings/settings-parser.h>

#include <string>

using namespace napa;

TEST_CASE("Parsing nothing doesn't fail", "[settings-parser]") {
    settings::PlatformSettings settings;

    REQUIRE(settings::ParseFromString("", settings));

    std::vector<char*> args = { "dummy.exe" };
    REQUIRE(settings::ParseFromConsole(static_cast<int>(args.size()), args.data(), settings));
}

TEST_CASE("Parsing from string", "[settings-parser]") {
    settings::PlatformSettings settings;
    settings.loggingProvider = "";

    REQUIRE(settings::ParseFromString("--loggingProvider myProvider", settings));
    REQUIRE(settings.loggingProvider == "myProvider");
}

TEST_CASE("Parsing from console", "[settings-parser]") {
    settings::PlatformSettings settings;
    settings.loggingProvider = "";

    std::vector<char*> args = { "dummy.exe", "--loggingProvider", "myProvider" };

    REQUIRE(settings::ParseFromConsole(static_cast<int>(args.size()), args.data(), settings));
    REQUIRE(settings.loggingProvider == "myProvider");
}

TEST_CASE("Parsing non existing setting fails", "[settings-parser]") {
    settings::PlatformSettings settings;

    REQUIRE(settings::ParseFromString("--thisSettingDoesNotExist noValue", settings) == false);
}

TEST_CASE("Parsing does not change defaults if setting is not provided", "[settings-parser]") {
    settings::PlatformSettings settings;
    settings.metricProvider = "myMetricProvider";

    REQUIRE(settings::ParseFromString("--loggingProvider myProvider", settings));
    REQUIRE(settings.metricProvider == "myMetricProvider");
}

TEST_CASE("Parsing with extra white spaces succeeds", "[settings-parser]") {
    settings::PlatformSettings settings;
    settings.loggingProvider = "";

    REQUIRE(settings::ParseFromString("  --loggingProvider \t   myProvider \t\t ", settings));
    REQUIRE(settings.loggingProvider == "myProvider");
}

TEST_CASE("Parsing with empty string succeeds", "[settings-parser]") {
    settings::PlatformSettings settings;

    REQUIRE(settings::ParseFromString("", settings) == true);
}

TEST_CASE("Parsing with different value type fails", "[settings-parser]") {
    settings::ZoneSettings settings;

    REQUIRE(settings::ParseFromString("--workers five", settings) == false);
}
