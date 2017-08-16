// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "stddef.h"
#include "stdint.h"

/// <summary> Simple non owning string. Should only be used for binding. </summary>
typedef struct {
    const char* data;
    size_t size;
} napa_string_ref;

#define NAPA_STRING_REF_WITH_SIZE(data, size) (napa_string_ref{ (data), (size) })
#define NAPA_STRING_REF(data) NAPA_STRING_REF_WITH_SIZE(data, strlen(data))

const napa_string_ref EMPTY_NAPA_STRING_REF = NAPA_STRING_REF_WITH_SIZE(0, 0);

#ifdef __cplusplus

namespace napa {
typedef napa_string_ref StringRef;
}

#define STD_STRING_TO_NAPA_STRING_REF(str) (napa_string_ref{ (str).data(), (str).size() })
#define NAPA_STRING_REF_TO_STD_STRING(str) (std::string((str).data, (str).size))

#endif // __cplusplus

/// <summary> Represents result code in napa zone apis. </summary>
#define NAPA_RESULT_CODE_DEF(symbol, ...) NAPA_RESULT_##symbol

typedef enum {

#include "napa/result-codes.inc"

} napa_result_code;

#undef NAPA_RESULT_CODE_DEF

#ifdef __cplusplus

namespace napa {
typedef napa_result_code ResultCode;
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

/// <summary> Represents options for calling a function. </summary>
typedef struct {

    /// <summary> Timeout in milliseconds - Use 0 for inifinite. </summary>
    uint32_t timeout;

    /// <summary> Arguments transport option. Default is AUTO. </summary>
    napa_transport_option transport;
} napa_zone_call_options;

#ifdef __cplusplus

namespace napa {
typedef napa_zone_call_options CallOptions;
}

#endif // __cplusplus

/// <summary> Represents a function to run within a zone, with binded arguments . </summary>
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
    napa_zone_call_options options;

    /// <summary> A context used for transporting handles across zones/workers. </summary>
    void* transport_context;
} napa_zone_function_spec;

#ifdef __cplusplus

#include <napa/transport/transport-context.h>
#include <memory>
#include <string>
#include <vector>

namespace napa {
/// <summary> Represents a function to call with its arguments. </summary>
struct FunctionSpec {

    /// <summary> The module that exports the function to execute. </summary>
    StringRef module = EMPTY_NAPA_STRING_REF;

    /// <summary> The function to execute. </summary>
    StringRef function = EMPTY_NAPA_STRING_REF;

    /// <summary> The function arguments. </summary>
    std::vector<StringRef> arguments;

    /// <summary> Execute options. </summary>
    CallOptions options = { 0, AUTO };

    /// <summary> Used for transporting shared_ptr and unique_ptr across zones/workers. </summary>
    mutable std::unique_ptr<napa::transport::TransportContext> transportContext;
};
}

#endif // __cplusplus

/// <summary> Represents a result from executing in a zone. </summary>
typedef struct {

    /// <summary> A result code. </summary>
    napa_result_code code;

    /// <summary> The error message in case of an error. </summary>
    napa_string_ref error_message;

    /// <summary> The return value in case of success. </summary>
    napa_string_ref return_value;

    /// <summary> A context used for transporting handles across zones/workers. </summary>
    void* transport_context;
} napa_zone_result;

#ifdef __cplusplus

namespace napa {
/// <summary> Represents a function call result. </summary>
struct Result {

    /// <summary> A result code. </summary>
    ResultCode code;

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
typedef void (*napa_zone_broadcast_callback)(napa_result_code code, void* context);
typedef void (*napa_zone_execute_callback)(napa_zone_result result, void* context);

#ifdef __cplusplus

#include <functional>

namespace napa {
typedef std::function<void(ResultCode)> BroadcastCallback;
typedef std::function<void(Result)> ExecuteCallback;
}

#endif // __cplusplus

/// <summary> Zone handle type. </summary>
typedef struct napa_zone* napa_zone_handle;

/// <summary> Callback for customized memory allocator. </summary>
typedef void* (*napa_allocate_callback)(size_t);
typedef void (*napa_deallocate_callback)(void*, size_t);
