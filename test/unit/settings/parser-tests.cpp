#include "catch.hpp"

#include "settings/settings-parser.h"

#include <string>

using namespace napa;

TEST_CASE("Parsing nothing doesn't fail", "[settings-parser]") {
    PlatformSettings settings;

    REQUIRE(settings_parser::ParseFromString("", settings));
    REQUIRE(settings_parser::ParseFromConsole(0, nullptr, settings));
}

TEST_CASE("Parsing from string", "[settings-parser]") {
    PlatformSettings settings;
    settings.workers = 1;
    settings.loggingProvider = "";

    REQUIRE(settings_parser::ParseFromString("--workers 5 --loggingProvider myProvider", settings));

    REQUIRE(settings.workers == 5);
    REQUIRE(settings.loggingProvider == "myProvider");
}

TEST_CASE("Parsing from console", "[settings-parser]") {
    PlatformSettings settings;
    settings.workers = 1;
    settings.loggingProvider = "";

    std::vector<char*> args = { "--workers", "5", "--loggingProvider", "myProvider" };

    REQUIRE(settings_parser::ParseFromConsole(static_cast<int>(args.size()), args.data(), settings));

    REQUIRE(settings.workers == 5);
    REQUIRE(settings.loggingProvider == "myProvider");
}

TEST_CASE("Parsing non existing setting fails", "[settings-parser]") {
    PlatformSettings settings;

    REQUIRE(settings_parser::ParseFromString("--thisSettingDoesNotExist noValue", settings) == false);
}

TEST_CASE("Parsing does not change defaults if setting is not provided", "[settings-parser]") {
    PlatformSettings settings;
    settings.workers = 2412;

    REQUIRE(settings_parser::ParseFromString("--loggingProvider myProvider", settings));
    
    REQUIRE(settings.workers == 2412);
}

TEST_CASE("Parsing with extra white spaces succeeds", "[settings-parser]") {
    PlatformSettings settings;
    settings.workers = 1;
    settings.loggingProvider = "";

    REQUIRE(settings_parser::ParseFromString(" --workers 5  --loggingProvider \t   myProvider \t\t ", settings));

    REQUIRE(settings.workers == 5);
    REQUIRE(settings.loggingProvider == "myProvider");
}

TEST_CASE("Parsing with multiple values for one setting", "[settings-parser]") {
    PlatformSettings settings;
    settings.workers = 1;
    settings.v8Flags = {};

    REQUIRE(settings_parser::ParseFromString("--v8Flags one two three --workers 5", settings));

    REQUIRE(settings.workers == 5);
    REQUIRE(settings.v8Flags.size() == 3);
    REQUIRE(settings.v8Flags[0] == "one");
    REQUIRE(settings.v8Flags[1] == "two");
    REQUIRE(settings.v8Flags[2] == "three");
}

TEST_CASE("Parsing with empty string succeeds", "[settings-parser]") {
    PlatformSettings settings;

    REQUIRE(settings_parser::ParseFromString("", settings) == true);
}

TEST_CASE("Parsing with different value type fails", "[settings-parser]") {
    PlatformSettings settings;

    REQUIRE(settings_parser::ParseFromString("--workers five", settings) == false);
}
