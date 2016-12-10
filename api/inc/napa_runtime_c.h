#ifndef NAPA_RUNTIME_C_H
#define NAPA_RUNTIME_C_H

#include "napa/common_c.h"
#include "napa/exports.h"


/// <summary>Simple static cstring and length</summary>
typedef struct {
    const char* data;
    uint32_t length;
} StringValue;

/// <summary>Represents the callback result</summary>
typedef struct {
    NAPA_RESULT resultCode;
    StringValue response;
    StringValue error;
} CallbackResult;

/// <summary>Callback signature</summary>
typedef void(*Callback)(CallbackResult result);

/// <summary>
///     Container handle type.
///     In C language there are no classes, so empty struct is used in order to ensure type safety.
/// </summary>
typedef struct {} *ContainerHandle;

/// <summary>Creates a napa container</summary>
EXTERN_C NAPA_API ContainerHandle CreateContainer();

/// <summary>
///     Initializes the napa container, providing specific settings.
///     The provided settings override any settings that were previously set.
///     TODO: specify public settings here
/// </summary>
/// <param name="handle">The container handle</param>
/// <param name="argc">Number of arguments</param>
/// <param name="argv">The arguments</param>
EXTERN_C NAPA_API NAPA_RESULT InitializeContainer(ContainerHandle handle, int argc, StringValue argv[]);

/// <summary>Sets an opaque pointer and it's associated key </summary>
/// <param name="handle">The container handle</param>
/// <param name="key">A unique identifier for the opaque value pointer</param>
/// <param name="value">
///     A void pointer that enables the caller to provide long lived objects that can be accessed using
///     a dedicated core module (TODO: Replace comment with real module name).
///     This value is kept available to be used as long as the container is not destroyed/released.
///     It is the responsibility of the caller to free its memory but it should not occur before the
///     container destruction.
/// </param>
EXTERN_C NAPA_API NAPA_RESULT SetGlobalValue(ContainerHandle handle, StringValue key, void* value);

/// <summary>Loads the content of the provided file into the container</summary>
/// <param name="handle">The container handle</param>
/// <param name="file">The path to the JavaScript file</param>
EXTERN_C NAPA_API NAPA_RESULT LoadFile(ContainerHandle handle, StringValue file);

/// <summary>Loads the provided source code into the container</summary>
/// <param name="handle">The container handle</param>
/// <param name="source">The JavaScript source code</param>
EXTERN_C NAPA_API NAPA_RESULT Load(ContainerHandle handle, StringValue source);

/// <summary>Runs a pre-loaded function (That was loaded using any Load API)</summary>
/// <param name="handle">The container handle</param>
/// <param name="func">The name of the function to run</param>
/// <param name="argc">The number of arguments that are to be passed to the function</param>
/// <param name="argv">The arguments</param>
/// <param name="callback">A callback that is triggered when execution is done</param>
/// <param name="timeout">Timeout in milliseconds - Use 0 for inifinite</param>
EXTERN_C NAPA_API NAPA_RESULT Run(ContainerHandle handle, StringValue func, int argc, StringValue argv[], Callback callback, uint32_t timeout);

/// <summary>Cleanup container resources and free its memory</summary>
/// <param name="handle">The container handle</param>
EXTERN_C NAPA_API NAPA_RESULT ReleaseContainer(ContainerHandle handle);


/// <summary>
///     Global napa initialization. Invokes initialization steps that are cross containers.
///     The settings passed represent the defaults for all the containers
///     but can be overriden in the container initialization API.
///     TODO: specify public settings here.
/// </summary>
/// <param name="handle">The container handle</param>
/// <param name="argc">Number of arguments</param>
/// <param name="argv">The arguments</param>
EXTERN_C NAPA_API NAPA_RESULT Initialize(int argc, StringValue argv[]);

/// <summary>Invokes napa shutdown steps. All non destoryed/released containers will be destroyed</summary>
EXTERN_C NAPA_API NAPA_RESULT Shutdown();

// TODO: Currently the logging and metric provider setters will only work for c++ compiled clients
//       Need to find a solution for pure c users (i.e. Java binding)
#ifdef __cplusplus
#include "napa/logging.h"
#include "napa/metric.h"

/// <summary>Allows the user to set a custom logging provider</summary>
EXTERN_C NAPA_API NAPA_RESULT SetLoggingProvider(napa::runtime::LoggingProvider* provider);

/// <summary>Allows the user to set a custom metric provider</summary>
EXTERN_C NAPA_API NAPA_RESULT SetMetricProvider(napa::runtime::MetricProvider* provider);

#endif // __cplusplus


#endif // NAPA_RUNTIME_C_H
