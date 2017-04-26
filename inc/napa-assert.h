#pragma once

#include <iostream>
#include <exception> 

#define NAPA_ASSERT(condition, message) do {            \
    if (!(condition)) {                                 \
        std::cerr << "Assertion failed: `"              \
                  << #condition                         \
                  << "`, file "                         \
                  << __FILE__                           \
                  << ", line "                          \
                  << __LINE__                           \
                  << " : "                              \
                  << message                            \
                  << "."                                \
                  << std::endl;                         \
        std::terminate();                               \
    }                                                   \
} while (false)

#define NAPA_FAIL(message) NAPA_ASSERT(false, message)