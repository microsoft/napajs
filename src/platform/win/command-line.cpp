#include <napa/module/command-line.h>
#include <windows.h>

using namespace napa::module;

int command_line::argc = __argc;
char** command_line::argv = __argv;