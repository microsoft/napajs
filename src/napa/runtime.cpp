#include "napa-runtime-c.h"

#include "container.h"


napa_container_handle napa_container_create()
{
    return reinterpret_cast<napa_container_handle>(new napa::runtime::internal::Container());
}

napa_response_code napa_container_init(
    napa_container_handle handle,
    napa_string_ref settings)
{
    return NAPA_RESPONSE_SUCCESS;
}

napa_response_code napa_container_set_global_value(
    napa_container_handle handle,
    napa_string_ref key,
    void* value)
{
    return NAPA_RESPONSE_SUCCESS;
}

napa_response_code napa_container_load_file(
    napa_container_handle handle,
    napa_string_ref file)
{
    return NAPA_RESPONSE_SUCCESS;
}

napa_response_code napa_container_load(
    napa_container_handle handle,
    napa_string_ref source)
{
    return NAPA_RESPONSE_SUCCESS;
}

napa_response_code napa_container_run(
    napa_container_handle handle,
    napa_string_ref func,
    uint32_t argc,
    napa_string_ref argv[],
    napa_container_callback callback,
    void* context,
    uint32_t timeout)
{
    return NAPA_RESPONSE_SUCCESS;
}

napa_container_response napa_container_run_sync(
    napa_container_handle handle,
    napa_string_ref func,
    uint32_t argc,
    napa_string_ref argv[],
    uint32_t timeout)
{
    return napa_container_response{ NAPA_RESPONSE_SUCCESS, CREATE_NAPA_STRING_REF("", 0) };
}

napa_response_code napa_container_release(napa_container_handle handle)
{
    return NAPA_RESPONSE_SUCCESS;
}

napa_response_code napa_initialize(napa_string_ref settings)
{
    return NAPA_RESPONSE_SUCCESS;
}

napa_response_code napa_initialize_from_console(
    int argc,
    char* argv[])
{
    return NAPA_RESPONSE_SUCCESS;
}

napa_response_code napa_shutdown()
{
    return NAPA_RESPONSE_SUCCESS;
}



#define NAPA_RESPONSE_CODE_DEF(symbol, string_rep) string_rep

/// <summary>Configuration file for Napa Engine.</summary>
const char* NAPA_RESPONSE_CODE_STRINGS[] = {
#include "napa/response-codes.inc"
};

#undef NAPA_RESPONSE_CODE_DEF

const char* napa_response_code_to_string(napa_response_code code)
{
    // TODO: assert code is in array boundaries

    return NAPA_RESPONSE_CODE_STRINGS[code];
}
