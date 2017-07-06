#pragma once

#if defined(_WIN32) || defined(__WIN32__)

    // Microsoft Windows

    // API exported from napa.dll
    #ifdef NAPA_EXPORTS
        #define NAPA_API __declspec(dllexport)
    #else
        #define NAPA_API __declspec(dllimport)
    #endif // NAPA_EXPORTS

    // API exported from napa-binding. (both napa.dll and napa-binding.node)
    #ifdef NAPA_BINDING_EXPORTS
        #define NAPA_BINDING_API __declspec(dllexport)
    #else
        #define NAPA_BINDING_API __declspec(dllimport)
    #endif // NAPA_BINDING_EXPORTS

#elif defined(linux) || defined(__linux)

    // Linux

    // API exported from napa.so
    #ifdef NAPA_EXPORTS
        #define NAPA_API __attribute__((visibility("default")))
    #else
        #define NAPA_API
    #endif // NAPA_EXPORTS

    // API exported from napa-binding. (both napa.so and napa-binding.node)
    #ifdef NAPA_BINDING_EXPORTS
        #define NAPA_BINDING_API __attribute__((visibility("default")))
    #else
        #define NAPA_BINDING_API
    #endif // NAPA_BINDING_EXPORTS

#else

    static_assert(false, "Unknown dynamic link import/export semantics");

#endif


#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif // __cplusplus
