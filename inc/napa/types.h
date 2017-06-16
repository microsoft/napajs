#pragma once

#include "stddef.h"
#include "stdint.h"

/// <summary> Simple non ownning string. Should only be used for binding. </summary>
typedef struct {
    const char* data;
    size_t size;
} napa_string_ref;

#define NAPA_STRING_REF_WITH_SIZE(data, size) (napa_string_ref { (data), (size) })
#define NAPA_STRING_REF(data) NAPA_STRING_REF_WITH_SIZE(data, strlen(data))

const napa_string_ref EMPTY_NAPA_STRING_REF = NAPA_STRING_REF_WITH_SIZE(0, 0);

#ifdef __cplusplus

namespace napa {
    typedef napa_string_ref StringRef;
}

#define STD_STRING_TO_NAPA_STRING_REF(str) (napa_string_ref { (str).data(), (str).size() })
#define NAPA_STRING_REF_TO_STD_STRING(str) (std::string((str).data, (str).size))

#endif // __cplusplus

/// <summary> Represents response code in napa zone apis. </summary>
#define NAPA_RESPONSE_CODE_DEF(symbol, ...) NAPA_RESPONSE_##symbol

typedef enum {

#include "napa/response-codes.inc"

} napa_response_code;

#undef NAPA_RESPONSE_CODE_DEF

#ifdef __cplusplus

namespace napa {
    typedef napa_response_code ResponseCode;
}

#endif // __cplusplus

/// <summary> Represents option for transporting objects in zone.execute. </summary>
typedef enum {

    /// <summary> 
    ///     transport.marshall/unmarshall will be done by `napajs` automatically.
    ///     This is the most common way, but may not be performance optimal with objects
    ///     that will be shared in multiple zone.execute.
    /// </summary>
    AUTO,

    /// <summary> transport.marshall/unmarshall will be done by user manually. </summary>
    MANUAL,
} napa_transport_option;

#ifdef __cplusplus

namespace napa {
    typedef napa_transport_option TransportOption;
}

#endif // __cplusplus

/// <summary> Represents options for an execution request. </summary>
typedef struct {

    /// <summary> Timeout in milliseconds - Use 0 for inifinite. </summary>
    uint32_t timeout;

    /// <summary> Arguments transport option. Default is AUTO. </summary>
    napa_transport_option transport;
} napa_zone_execute_options;

#ifdef __cplusplus

namespace napa {
    typedef napa_zone_execute_options ExecuteOptions;
}

#endif // __cplusplus

/// <summary> Represents an execution request for a zone. </summary>
typedef struct {

    /// <summary> The module that exports the function to execute. </summary>
    napa_string_ref module;

    /// <summary> The function to execute. </summary>
    napa_string_ref function;

    /// <summary> The function arguments. </summary>
    const napa_string_ref* arguments;

    /// <summary> The number of arguments. </summary>
    size_t arguments_count;

    /// <summary> Options. </summary>
    napa_zone_execute_options options;

    /// <summary> A context used for transporting handles across zones/workers. </summary>
    void* transport_context;
} napa_zone_execute_request;

#ifdef __cplusplus

#include <napa/transport/transport-context.h>
#include <memory>
#include <string>
#include <vector>

namespace napa {
    /// <summary> Represents an execution request. </summary>
    struct ExecuteRequest {

        /// <summary> The module that exports the function to execute. </summary>
        StringRef module = EMPTY_NAPA_STRING_REF;

        /// <summary> The function to execute. </summary>
        StringRef function = EMPTY_NAPA_STRING_REF;

        /// <summary> The function arguments. </summary>
        std::vector<StringRef> arguments;

        /// <summary> Execute options. </summary>
        ExecuteOptions options = { 0, AUTO };

        /// <summary> Used for transporting shared_ptr and unique_ptr across zones/workers. </summary>
        mutable std::unique_ptr<napa::transport::TransportContext> transportContext;
    };
}

#endif // __cplusplus

/// <summary> Represents a response from executing in a zone. </summary>
typedef struct {

    /// <summary> A response code. </summary>
    napa_response_code code;

    /// <summary> The error message in case of an error. </summary>
    napa_string_ref error_message;

    /// <summary> The return value in case of success. </summary>
    napa_string_ref return_value;

    /// <summary> A context used for transporting handles across zones/workers. </summary>
    void* transport_context;
} napa_zone_execute_response;

#ifdef __cplusplus

namespace napa {
    /// <summary> Represents an execution response. </summary>
    struct ExecuteResponse {

        /// <summary> A response code. </summary>
        ResponseCode code;

        /// <summary> The error message in case of an error. </summary>
        std::string errorMessage;

        /// <summary> The return value in case of success. </summary>
        std::string returnValue;

        /// <summary> Used for transporting shared_ptr and unique_ptr across zones/workers. </summary>
        mutable std::unique_ptr<napa::transport::TransportContext> transportContext;
    };
}

#endif // __cplusplus

/// <summary> Callback signatures. </summary>
typedef void(*napa_zone_broadcast_callback)(napa_response_code code, void* context);
typedef void(*napa_zone_execute_callback)(napa_zone_execute_response response, void* context);

#ifdef __cplusplus

#include <functional>

namespace napa {
    typedef std::function<void(ResponseCode)> BroadcastCallback;
    typedef std::function<void(ExecuteResponse)> ExecuteCallback;
}

#endif // __cplusplus

/// <summary> Zone handle type. </summary>
typedef struct napa_zone *napa_zone_handle;

/// <summary> Callback for customized memory allocator. </summary>
typedef void* (*napa_allocate_callback)(size_t);
typedef void (*napa_deallocate_callback)(void*, size_t);
