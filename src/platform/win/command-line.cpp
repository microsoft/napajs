#include <napa/module/command-line.h>

#include <sstream>
#include <windows.h>

using namespace napa::module;

int command_line::argc = __argc;
char** command_line::argv = __argv;
const char* command_line::ENV_DELIMITER = ";";

bool command_line::SetEnv(const char* name, const char* value) {
    std::ostringstream oss;
    oss << name << "=" << value;
    return _putenv(oss.str().c_str()) == 0;
}

std::string command_line::GetEnv(const char* name) {
    std::string value;

    char* buffer = nullptr;
    size_t size = 0;
    if (_dupenv_s(&buffer, &size, name) == 0 && buffer != nullptr) {
        value.assign(buffer, size);
        free(buffer);
    }

    return value;
}
