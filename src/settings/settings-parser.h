#pragma once

#include "settings.h"

#include <string>

namespace napa {
namespace runtime {
namespace internal {
namespace settings_parser {

    /// <summary> Parses napa settings from string. </summary>
    /// <param name="str"> The settings string that will be parsed. </param>
    /// <param name="settings">
    ///     A setting out parameter that is filled with parsed settings.
    ///     Settings that do not appear in the settings string will not be changed.
    /// </param>
    /// <returns> True if parsing succeeded, false otherwise. </returns>
    bool ParseFromString(const std::string& str, Settings& settings);

    /// <summary> Parses napa settings console args. </summary>
    /// <param name="argc"> Number of arguments. </param>
    /// <param name="argv"> The arguments. </param>
    /// <param name="settings">
    ///     A setting out parameter that is filled with parsed settings.
    ///     Settings that do not appear in the settings string will not be changed.
    /// </param>
    /// <returns> True if parsing succeeded, false otherwise. </returns>
    bool ParseFromConsole(int argc, char* argv[], Settings& settings);
 
}
}
}
}