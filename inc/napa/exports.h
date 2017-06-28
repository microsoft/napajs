#pragma once

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

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif // __cplusplus