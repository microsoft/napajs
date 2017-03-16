#pragma once

#include <string>

namespace napa {
namespace module {
namespace command_line {

    /// <summary> Global variable to indicate the number of process arguments. </summary>
    extern int argc;

    /// <summary> Global variable to indicate process arguments. </summary>
    extern char** argv;

    /// <summary> Environment variables delimiter. </summary>
    extern const char* ENV_DELIMITER;

    /// <summary> Set environment variable. </summary>
    bool SetEnv(const char* name, const char* value);

    /// <summary> Get environment variable. </summary>
    std::string GetEnv(const char* name);

}   // End of namespce command_line.
}   // End of namespce module
}   // End of namespce napa.
