#pragma once

#include <string>

namespace napa {
namespace platform {

    /// <summary> Global variable to indicate the number of process arguments. </summary>
    extern int argc;

    /// <summary> Global variable to indicate process arguments. </summary>
    extern char** argv;

    /// <summary> Platform. </summary>
    extern const char* PLATFORM;

    /// <summary> OS type. </summary>
    extern const char* OS_TYPE;

    /// <summary> Environment variables delimiter. </summary>
    extern const char* ENV_DELIMITER;

    /// <summary> Directory separator. </summary>
    extern const char* DIR_SEPARATOR;

    /// <summary> Set environment variable. </summary>
    bool SetEnv(const char* name, const char* value);

    /// <summary> Get environment variable. </summary>
    std::string GetEnv(const char* name);

    /// <summary> Get the full path of process. </summary>
    std::string GetExecPath();

    /// <summary> Set file permission mask at the current process. </summary>
    int32_t Umask(int32_t mode);

    /// <summary> Return pid. </summary>
    int32_t Getpid();

    /// <summary> Return tid. </summary>
    int32_t Gettid();

    /// <summary> Return nonzero value if a descriptor is associated with a character device. </summary>
    /// <param name="fd"> File descriptor. </param>
    int32_t Isatty(int32_t fd);

}   // End of namespce platform.
}   // End of namespce napa.
