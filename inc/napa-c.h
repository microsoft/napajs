#pragma once

#include "napa/exports.h"
#include "napa/common.h"

/// <summary> Represents napa response. </summary>
typedef struct {
    napa_response_code code;
    napa_string_ref error_message;
    napa_string_ref return_value;
} napa_container_response;

/// <summary> Callback signatures. </summary>
typedef void(*napa_container_run_callback)(napa_container_response response, void* context);
typedef void(*napa_container_load_callback)(napa_response_code code, void* context);

/// <summary>
///     Container handle type.
///     In C language there are no classes, so empty struct is used in order to ensure type safety.
/// </summary>
typedef struct {} *napa_container_handle;

/// <summary> Creates a napa container. </summary>
EXTERN_C NAPA_API napa_container_handle napa_container_create();

/// <summary>
///     Initializes the napa container, providing specific settings.
///     The provided settings override any settings that were previously set.
///     TODO: specify public settings here
/// </summary>
/// <param name="handle"> The container handle. </param>
/// <param name="settings"> The settings string. </param>
EXTERN_C NAPA_API napa_response_code napa_container_init(
    napa_container_handle handle,
    napa_string_ref settings);

/// <summary> Sets an opaque pointer and it's associated key. </summary>
/// <param name="handle"> The container handle. </param>
/// <param name="key"> A unique identifier for the opaque value pointer. </param>
/// <param name="value">
///     A void pointer that enables the caller to provide long lived objects that can be accessed
///     using a dedicated core module (TODO: Replace comment with real module name).
///     This value is kept available to be used as long as the container is not destroyed/released.
///     It is the responsibility of the caller to free its memory but it should not occur before
///     the container destruction.
/// </param>
EXTERN_C NAPA_API napa_response_code napa_container_set_global_value(
    napa_container_handle handle,
    napa_string_ref key,
    void* value);

/// <summary> Loads the content of the provided file into the container asynchronously. </summary>
/// <param name="handle"> The container handle. </param>
/// <param name="file"> The path to the JavaScript file. </param>
/// <param name="callback"> A callback that is triggered when loading is done. </param>
/// <param name="context"> An opaque pointer that is passed back in the callback. </param>
EXTERN_C NAPA_API void napa_container_load_file(
    napa_container_handle handle,
    napa_string_ref file,
    napa_container_load_callback callback,
    void* context);

/// <summary> Loads the provided source code into the container asynchronously. </summary>
/// <param name="handle"> The container handle. </param>
/// <param name="source"> The JavaScript source code. </param>
/// <param name="callback"> A callback that is triggered when loading is done. </param>
/// <param name="context"> An opaque pointer that is passed back in the callback. </param>
EXTERN_C NAPA_API void napa_container_load(
    napa_container_handle handle,
    napa_string_ref source,
    napa_container_load_callback callback,
    void* context);

/// <summary> Runs a pre-loaded function asynchronously. </summary>
/// <param name="handle"> The container handle. </param>
/// <param name="func"> The name of the function to run. </param>
/// <param name="argc"> The number of arguments that are to be passed to the function. </param>
/// <param name="argv"> The arguments. </param>
/// <param name="callback"> A callback that is triggered when execution is done. </param>
/// <param name="context"> An opaque pointer that is passed back in the callback. </param>
/// <param name="timeout"> Timeout in milliseconds - Use 0 for inifinite. </param>
EXTERN_C NAPA_API void napa_container_run(
    napa_container_handle handle,
    napa_string_ref func,
    size_t argc,
    const napa_string_ref argv[],
    napa_container_run_callback callback,
    void* context,
    uint32_t timeout);

/// <summary> Cleanup container resources and free its memory. </summary>
/// <param name="handle"> The container handle. </param>
EXTERN_C NAPA_API napa_response_code napa_container_release(napa_container_handle handle);

/// <summary>
///     Global napa initialization. Invokes initialization steps that are cross containers.
///     The settings passed represent the defaults for all the containers
///     but can be overriden in the container initialization API.
///     TODO: specify public settings here.
/// </summary>
/// <param name="handle"> The container handle. </param>
/// <param name="settings"> The settings string. </param>
EXTERN_C NAPA_API napa_response_code napa_initialize(napa_string_ref settings);

/// <summary>
///     Same as napa_initialize only accepts arguments as provided by console
/// </summary>
/// <param name="handle"> The container handle. </param>
/// <param name="argc"> Number of arguments. </param>
/// <param name="argv"> The arguments. </param>
EXTERN_C NAPA_API napa_response_code napa_initialize_from_console(
    int argc,
    char* argv[]);

/// <summary> Invokes napa shutdown steps. All non released containers will be destroyed. </summary>
EXTERN_C NAPA_API napa_response_code napa_shutdown();

/// <summary> Convert the napa response code to its string representation. </summary>
/// <param name="code"> The response code. </param>
EXTERN_C NAPA_API const char* napa_response_code_to_string(napa_response_code code);