// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <platform/process.h>
#include <platform/platform.h>

#ifdef SUPPORT_POSIX

#include <unistd.h>
#include <limits.h>
#include <sys/syscall.h>

#else

#include <windows.h>

#include <io.h>
#include <process.h>

#endif

#include <sys/stat.h>

#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace napa {
namespace platform {

namespace {
#ifdef SUPPORT_POSIX
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
}

int GetArgc() {
#ifdef SUPPORT_POSIX
    return GetCommandLineArgs().argc;
#else
    return __argc;
#endif
}

/// <summary> Get the process arguments. </summary>
char** GetArgv() {
#ifdef SUPPORT_POSIX
    return const_cast<char**>(&GetCommandLineArgs().argv[0]);
#else
    return __argv;
#endif
}

bool SetEnv(const char* name, const char* value) {
#ifdef SUPPORT_POSIX
    return 0 == setenv(name, value, 1);
#else
    std::ostringstream oss;
    oss << name << "=" << value;
    return _putenv(oss.str().c_str()) == 0;
#endif
}

std::string GetEnv(const char* name) {
#ifdef SUPPORT_POSIX
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

int32_t Umask(int32_t mode) {
#ifdef SUPPORT_POSIX
    return static_cast<int32_t>(umask(mode));
#else
    int32_t oldMask;
    if (_umask_s(mode, &oldMask)) {
        throw std::runtime_error("Error setting umask");
    }
    return oldMask;
#endif
}

int32_t Getpid() {
#ifdef SUPPORT_POSIX
    return static_cast<int32_t>(getpid());
#else
    return _getpid();
#endif
}

int32_t Gettid() {
#ifdef SUPPORT_POSIX
    return static_cast<int32_t>(syscall(SYS_gettid));
#else
    return static_cast<int32_t>(GetCurrentThreadId());
#endif
}

int32_t Isatty(int32_t fd) {
#ifdef SUPPORT_POSIX
    return static_cast<int32_t>(isatty(fd));
#else
    return _isatty(fd);
#endif
}

}
}
