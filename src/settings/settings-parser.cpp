#include "settings-parser.h"

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

#include <iostream>

using namespace boost::program_options;
using namespace napa::runtime::internal;

static bool Parse(const std::vector<std::string>& args, Settings& settings) {
    // All parsing options should be added here.
    options_description desc;
    desc.add_options()
        ("cores", value(&settings.cores), "number of cores")
        ("v8Flags", value(&settings.v8Flags)->multitoken(), "v8 flags")
        ("loggingProvider", value(&settings.loggingProvider), "logging provider");

    try {
        variables_map vm;
        store(command_line_parser(args).options(desc).run(), vm);
        notify(vm);
    } catch (std::exception& ex) {
        std::cerr << "Failed to parse settings. Error: " << ex.what() << std::endl;
        return false;
    }

    return true;
}

bool settings_parser::ParseFromString(const std::string& str, Settings& settings) {
    std::vector<std::string> args;

    try {
        boost::split(args, str, boost::is_any_of("\t "), boost::token_compress_on);
    } catch (std::exception& ex) {
        std::cerr << "Failed to split input string [" << str << "] error: " << ex.what() << std::endl;
        return false;
    }

    return Parse(args, settings);
}

bool settings_parser::ParseFromConsole(int argc, char* argv[], Settings& settings) {
    std::vector<std::string> args;

    for (auto i = 0; i < argc; i++) {
        args.emplace_back(argv[i]);
    }

    return Parse(args, settings);
}
