// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

// Microsoft Windows
#if defined(_WIN32) || defined(__WIN32__)
#define DLL_EXPORT __declspec(dllexport)
#define DLL_IMPORT __declspec(dllimport)
// Linux
#elif defined(__GNUC__)
#define DLL_EXPORT __attribute__((visibility("default")))
#define DLL_IMPORT
#else
static_assert(false, "Unknown dynamic link import/export semantics");
#endif

// API exported from napa.dll
#ifdef NAPA_EXPORTS
#define NAPA_API DLL_EXPORT
#else
#define NAPA_API DLL_IMPORT
#endif // NAPA_EXPORTS

// API exported from napa-binding. (both napa.dll and napa-binding.node)
#ifdef NAPA_BINDING_EXPORTS
#define NAPA_BINDING_API DLL_EXPORT
#else
#define NAPA_BINDING_API DLL_IMPORT
#endif // NAPA_BINDING_EXPORTS

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif // __cplusplus
