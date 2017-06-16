#pragma once

#include "napa/exports.h"
#include "napa/types.h"

/// <summary> Creates a napa zone. </summary>
/// <param name="id"> A unique id for the zone. </param>
/// <remarks>
///     This function returns a handle that must be release when it's no longer needed.
///     Napa keeps track of all zone handles and destroys the zone when all handles have been released.
/// </remarks>
EXTERN_C NAPA_API napa_zone_handle napa_zone_create(napa_string_ref id);

/// <summary> Retrieves a zone by id. </summary>
/// <param name="id"> A unique id for the zone. </param>
/// <returns> The zone handle if exists, null otherwise. </returns>
/// <remarks>
///     This function returns a handle that must be release when it's no longer needed.
///     Napa keeps track of all zone handles and destroys the zone when all handles have been released.
/// </remarks>
EXTERN_C NAPA_API napa_zone_handle napa_zone_get(napa_string_ref id);

/// <summary> Retrieves the current zone. </summary>
/// <returns> The zone handle if this thread is associated with one, null otherwise. </returns>
/// <remarks>
///     This function returns a handle that must be release when it's no longer needed.
///     Napa keeps track of all zone handles and destroys the zone when all handles have been released.
/// </remarks>
EXTERN_C NAPA_API napa_zone_handle napa_zone_get_current();

/// <summary> Releases the zone handle. When all handles for a zone are released the zone is destoryed. </summary>
/// <param name="handle"> The zone handle. </param>
EXTERN_C NAPA_API napa_response_code napa_zone_release(napa_zone_handle handle);

/// <summary>
///     Initializes the napa zone, providing specific settings.
///     The provided settings override any settings that were previously set.
///     TODO: specify public settings here
/// </summary>
/// <param name="handle"> The zone handle. </param>
/// <param name="settings"> The settings string. </param>
EXTERN_C NAPA_API napa_response_code napa_zone_init(
    napa_zone_handle handle,
    napa_string_ref settings);

/// <summary> Retrieves the zone id. </summary>
/// <param name="handle"> The zone handle. </param>
EXTERN_C NAPA_API napa_string_ref napa_zone_get_id(napa_zone_handle handle);

/// <summary> Compiles and run the provided source code on all zone workers. </summary>
/// <param name="handle"> The zone handle. </param>
/// <param name="source"> The JavaScript source code. </param>
/// <param name="callback"> A callback that is triggered when broadcast is done. </param>
/// <param name="context"> An opaque pointer that is passed back in the callback. </param>
EXTERN_C NAPA_API void napa_zone_broadcast(
    napa_zone_handle handle,
    napa_string_ref source,
    napa_zone_broadcast_callback callback,
    void* context);

/// <summary> Executes a pre-loaded function asynchronously in a single zone wroker. </summary>
/// <param name="handle"> The zone handle. </param>
/// <param name="request"> The execution request. </param>
/// <param name="callback"> A callback that is triggered when execution is done. </param>
/// <param name="context"> An opaque pointer that is passed back in the callback. </param>
EXTERN_C NAPA_API void napa_zone_execute(
    napa_zone_handle handle,
    napa_zone_execute_request request,
    napa_zone_execute_callback callback,
    void* context);

/// <summary>
///     Global napa initialization. Invokes initialization steps that are cross zones.
///     The settings passed represent the defaults for all the zones
///     but can be overriden in the zone initialization API.
///     TODO: specify public settings here.
/// </summary>
/// <param name="settings"> The settings string. </param>
EXTERN_C NAPA_API napa_response_code napa_initialize(napa_string_ref settings);

/// <summary>
///     Same as napa_initialize only accepts arguments as provided by console
/// </summary>
/// <param name="argc"> Number of arguments. </param>
/// <param name="argv"> The arguments. </param>
EXTERN_C NAPA_API napa_response_code napa_initialize_from_console(
    int argc,
    char* argv[]);

/// <summary> Invokes napa shutdown steps. All non released zones will be destroyed. </summary>
EXTERN_C NAPA_API napa_response_code napa_shutdown();

/// <summary> Convert the napa response code to its string representation. </summary>
/// <param name="code"> The response code. </param>
EXTERN_C NAPA_API const char* napa_response_code_to_string(napa_response_code code);

/// <summary> Set customized allocator, which will be used for napa_allocate and napa_deallocate.
/// If user doesn't call napa_allocator_set, C runtime malloc/free from napa.dll will be used. </summary>
/// <param name="allocate_callback"> Function pointer for allocating memory, which should be valid during the entire process. </param>
/// <param name="deallocate_callback"> Function pointer for deallocating memory, which should be valid during the entire process. </param>
EXTERN_C NAPA_API void napa_allocator_set(
    napa_allocate_callback allocate_callback, 
    napa_deallocate_callback deallocate_callback);

/// <summary> Allocate memory using napa allocator from napa_allocator_set, which is using C runtime ::malloc if not called. </summary>
/// <param name="size"> Size of memory requested in byte. </param>
/// <returns> Allocated memory. </returns>
EXTERN_C NAPA_API void* napa_allocate(size_t size);

/// <summary> Free memory using napa allocator from napa_allocator_set, which is using C runtime ::free if not called. </summary>
/// <param name="pointer"> Pointer to memory to be freed. </param>
/// <param name="size_hint"> Hint of size to deallocate. </param>
EXTERN_C NAPA_API void napa_deallocate(void* pointer, size_t size_hint);

/// <summary> Allocate memory using C runtime ::malloc from napa.dll. </summary>
/// <param name="size"> Size of memory requested in byte. </param>
/// <returns> Allocated memory. </returns>
EXTERN_C NAPA_API void* napa_malloc(size_t size);

/// <summary> Free memory using C runtime ::free from napa.dll. </summary>
/// <param name="pointer"> Pointer to memory to be freed. </param>
/// <param name="size_hint"> Hint of size to deallocate. </param>
EXTERN_C NAPA_API void napa_free(void* pointer, size_t size_hint);
