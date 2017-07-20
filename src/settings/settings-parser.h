// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "settings.h"
#include <utils/string.h>

#include <iostream>
#include <string>
#include <vector>

namespace napa {
namespace settings {

    /// <summary> Parses napa settings from a vector of arguments. </summary>
    /// <param name="str"> The arguments holding the settings. </param>
    /// <param name="settings">
    ///     A setting out parameter that is filled with parsed settings.
    ///     Settings that do not appear in the settings string will not be changed.
    /// </param>
    /// <returns> True if parsing succeeded, false otherwise. </returns>
    bool Parse(const std::vector<std::string>& args, ZoneSettings& settings);
    bool Parse(const std::vector<std::string>& args, PlatformSettings& settings);

    /// <summary> Parses napa settings from string. </summary>
    /// <param name="str"> The settings string that will be parsed. </param>
    /// <param name="settings">
    ///     A setting out parameter that is filled with parsed settings.
    ///     Settings that do not appear in the settings string will not be changed.
    /// </param>
    /// <returns> True if parsing succeeded, false otherwise. </returns>
    template <typename SettingsType>
    bool ParseFromString(const std::string& str, SettingsType& settings) {
        auto strCopy = utils::string::TrimCopy(str);
        if (strCopy.empty()) {
            return true;
        }

        std::vector<std::string> args;
        try {
            utils::string::Split(strCopy, args, "\t ", true);
        }
        catch (std::exception& ex) {
            std::cerr << "Failed to split input string [" << strCopy << "] error: " << ex.what() << std::endl;
            return false;
        }

        return Parse(args, settings);
    }

    /// <summary> Parses napa settings console args. </summary>
    /// <param name="argc"> Number of arguments. </param>
    /// <param name="argv"> The arguments. </param>
    /// <param name="settings">
    ///     A setting out parameter that is filled with parsed settings.
    ///     Settings that do not appear in the settings string will not be changed.
    /// </param>
    /// <returns> True if parsing succeeded, false otherwise. </returns>
    template <typename SettingsType>
    bool ParseFromConsole(int argc, const char* argv[], SettingsType& settings) {
        std::vector<std::string> args;

        args.reserve(argc);
        for (auto i = 0; i < argc; i++) {
            args.emplace_back(argv[i]);
        }

        return Parse(args, settings);
    }
}
}