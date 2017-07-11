#pragma once

#if defined(__POSIX__)

#include <limits.h>
#include <sys/syscall.h>
#include <unistd.h>

#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)

#include <io.h>
#include <process.h>
#include <windows.h>

#else

#error platform not supported

#endif

#include <sys/stat.h>
#include <sys/types.h>

#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace napa {
namespace platform {

#ifdef __POSIX__
    static size_t ReadBuffer(const char* filename, char* buffer, size_t length) {
        FILE* source = fopen(filename, "rb");
        if (source == nullptr) {
            return 0;
        }

        std::unique_ptr<FILE, std::function<void(FILE*)>> deferred(source, [](auto file) {
            fclose(file);
        });

        return fread(buffer, 1, length, source);
    }

    struct CommandLineArgs {
        int argc;
        std::vector<char*> argv;

        CommandLineArgs() {
            size_t length = ReadBuffer("/proc/self/cmdline", commandLine, PATH_MAX);
            commandLine[length] = '\0';

            ParseCommandLine(length);
        }

    private:
        void ParseCommandLine(size_t length) {
            argc = 0;
            argv.clear();

            bool start = true;
            for (size_t i = 0; i < length; i++) {
                if (start && commandLine[i] != '\0') {
                    start = false;
                    argv.push_back(commandLine + i);
                    argc++;
                }
                else if (commandLine[i] == '\0') {
                    start = true;
                }
            }
        }

        char commandLine[PATH_MAX + 1];
    };

    static const CommandLineArgs& GetCommandLineArgs() {
        static CommandLineArgs commandLineArgs;
        return commandLineArgs;
    }
#endif

    /// <summary> Platform. </summary>
#if defined(__linux) || defined(__linux__) || defined(linux)
    constexpr char* PLATFORM = "linux";
#elif defined(__APPLE__)
    constexpr char* PLATFORM = "darwin";
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    constexpr char* PLATFORM = "win32";
#elif defined(__FreeBSD__) || defined(__FreeBSD)
    constexpr char* PLATFORM = "freebsd";
#else
    constexpr char* PLATFORM = "unknown";
#endif

    /// <summary> Get OS type. </summary>
    inline const char* GetOSType() {
#ifdef __POSIX__
        struct utsname info;
        if (uname(&info) < 0) {
            throw std::runtime_error("Error getting uname");
        }
        return info.sysname;
#else
        return "Windows_NT";
#endif
    }

    /// <summary> Environment variables delimiter. </summary>
#ifdef __POSIX__
    constexpr char* ENV_DELIMITER = ":";
#else
    constexpr char* ENV_DELIMITER = ";";
#endif

    /// <summary> Directory separator. </summary>
#ifdef __POSIX__
    constexpr char* DIR_SEPARATOR = "/";
#else
    constexpr char* DIR_SEPARATOR = "\\";
#endif

    /// <summary> Get the number of process arguments. </summary>
    inline int GetArgc() {
#ifdef __POSIX__
        return GetCommandLineArgs().argc;
#else
        return __argc;
#endif
    }

    /// <summary> Get the process arguments. </summary>
    inline char** GetArgv() {
#ifdef __POSIX__
        return const_cast<char**>(&GetCommandLineArgs().argv[0]);
#else
        return __argv;
#endif
    }

    /// <summary> Set environment variable. </summary>
    inline bool SetEnv(const char* name, const char* value) {
#ifdef __POSIX__
        return 0 == setenv(name, value, 1);
#else
        std::ostringstream oss;
        oss << name << "=" << value;
        return _putenv(oss.str().c_str()) == 0;
#endif
    }

    /// <summary> Get environment variable. </summary>
    inline std::string GetEnv(const char* name) {
#ifdef __POSIX__
        std::string value;

        char* buffer = getenv(name);
        if (buffer != nullptr) {
            value.assign(buffer);
        }

        return value;
#else
        std::string value;

        char* buffer = nullptr;
        size_t size = 0;
        if (_dupenv_s(&buffer, &size, name) == 0 && buffer != nullptr) {
            value.assign(buffer, size - 1);
            free(buffer);
        }

        return value;
#endif
    }

    /// <summary> Get the full path of process. </summary>
    inline std::string GetExecPath() {
#ifdef __POSIX__
        char buffer[PATH_MAX];
        size_t size = ReadBuffer("/proc/self/exe", buffer, PATH_MAX);
        return std::string(buffer);
#else
        char buffer[MAX_PATH];
        GetModuleFileName(nullptr, buffer, MAX_PATH);
        return std::string(buffer);
#endif
    }

    /// <summary> Set file permission mask at the current process. </summary>
    inline int32_t Umask(int32_t mode) {
#ifdef __POSIX__
        return static_cast<int32_t>(umask(mode));
#else
        int32_t oldMask;
        if (_umask_s(mode, &oldMask)) {
            throw std::runtime_error("Error setting umask");
        }
        return oldMask;
#endif
    }

    /// <summary> Return pid. </summary>
    inline int32_t Getpid() {
#ifdef __POSIX__
        return static_cast<int32_t>(getpid());
#else
        return _getpid();
#endif
    }

    /// <summary> Return tid. </summary>
    inline int32_t Gettid() {
#ifdef __POSIX__
        return static_cast<int32_t>(syscall(SYS_gettid));
#else
        return static_cast<int32_t>(GetCurrentThreadId());
#endif
    }

    /// <summary> Return nonzero value if a descriptor is associated with a character device. </summary>
    /// <param name="fd"> File descriptor. </param>
    inline int32_t Isatty(int32_t fd) {
#ifdef __POSIX__
        return static_cast<int32_t>(isatty(fd));
#else
        return _isatty(fd);
#endif
    }

}   // End of namespce platform.
}   // End of namespce napa.
