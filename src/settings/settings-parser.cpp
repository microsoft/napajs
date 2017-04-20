#include "settings-parser.h"

#include <napa-log.h>

#include <boost/program_options.hpp>

using namespace boost::program_options;
using namespace napa;

static void AddZoneOptions(options_description& desc, ZoneSettings& settings) {
    // Zone parsing options should be added here.
    desc.add_options()
        ("workers", value(&settings.workers), "number of workers")
        ("bootstrapFile", value(&settings.bootstrapFile), "bootstrap file")
        ("maxOldSpaceSize", value(&settings.maxOldSpaceSize), "max old space size in MB")
        ("maxSemiSpaceSize", value(&settings.maxSemiSpaceSize), "max semi space size in MB")
        ("maxExecutableSize", value(&settings.maxExecutableSize), "max executable size in MB")
        ("maxStackSize", value(&settings.maxStackSize), "max isolate stack size in bytes");
}

static void AddPlatformOptions(options_description& desc, PlatformSettings& settings) {
    // Platform parsing options should be added here.
    desc.add_options()
        ("loggingProvider", value(&settings.loggingProvider), "logging provider")
        ("metricProvider", value(&settings.metricProvider), "metric provider")
        ("v8Flags", value(&settings.v8Flags)->multitoken(), "v8 flags")
        ("initV8", value(&settings.initV8), "specify whether v8 should be initialized");
}

bool settings_parser::Parse(const std::vector<std::string>& args, ZoneSettings& settings) {
    options_description desc;
    AddZoneOptions(desc, settings);

    try {
        variables_map vm;
        store(command_line_parser(args).options(desc).run(), vm);
        notify(vm);
    } catch (std::exception& ex) {
        std::cerr << "Failed to parse zone settings. Error: " << ex.what() << std::endl;
        return false;
    }

    NAPA_ASSERT(settings.workers > 0, "The number of workers must be greater than 0");
    NAPA_ASSERT(settings.maxStackSize > 0, "The maximum allowed stack size must be greater than 0");

    return true;
}

bool settings_parser::Parse(const std::vector<std::string>& args, PlatformSettings& settings) {
    options_description desc;
    AddZoneOptions(desc, settings);
    AddPlatformOptions(desc, settings);

    try {
        variables_map vm;
        store(command_line_parser(args).options(desc).run(), vm);
        notify(vm);
    }
    catch (std::exception& ex) {
        std::cerr << "Failed to parse platform settings. Error: " << ex.what() << std::endl;
        return false;
    }

    return true;
}