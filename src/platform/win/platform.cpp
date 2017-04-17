#include <napa/module/platform.h>

#include <sstream>

#include <io.h>
#include <process.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <windows.h>

using namespace napa::module;

int platform::argc = __argc;
char** platform::argv = __argv;
const char* platform::ENV_DELIMITER = ";";
const char* platform::PLATFORM = "win32";
const char* platform::OS_TYPE = "Windows_NT";
const char* platform::DIR_SEPARATOR = "\\";

bool platform::SetEnv(const char* name, const char* value) {
    std::ostringstream oss;
    oss << name << "=" << value;
    return _putenv(oss.str().c_str()) == 0;
}

std::string platform::GetEnv(const char* name) {
    std::string value;

    char* buffer = nullptr;
    size_t size = 0;
    if (_dupenv_s(&buffer, &size, name) == 0 && buffer != nullptr) {
        value.assign(buffer, size - 1);
        free(buffer);
    }

    return value;
}

std::string platform::GetExecPath() {
    char buffer[MAX_PATH];
    GetModuleFileName(nullptr, buffer, MAX_PATH);
    return std::string(buffer);
}

int32_t platform::Umask(int32_t mode) {
    return _umask(mode);
}

int32_t platform::Getpid() {
    return _getpid();
}

int32_t platform::Isatty(int32_t fd) {
    return _isatty(fd);
}