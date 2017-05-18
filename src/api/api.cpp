#include <napa-c.h>

#include "providers/providers.h"
#include "settings/settings-parser.h"
#include "v8/v8-common.h"
#include "zone/zone-impl.h"

#include <napa-log.h>

#include <boost/filesystem.hpp>

#include <atomic>
#include <fstream>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <vector>


using namespace napa;

static std::atomic<bool> _initialized(false);
static PlatformSettings _platformSettings;

/// <summary> a simple wrapper around Zone for managing lifetime using shared_ptr. </summary>
struct napa_zone {
    std::string id;
    std::shared_ptr<Zone> zone;
};

napa_zone_handle napa_zone_create(napa_string_ref id) {
    NAPA_ASSERT(_initialized, "Napa wasn't initialized");

    // The actual zone is created upon initialization.
    return new napa_zone { NAPA_STRING_REF_TO_STD_STRING(id), nullptr };
}

napa_zone_handle napa_zone_get(napa_string_ref id) {
    NAPA_ASSERT(_initialized, "Napa wasn't initialized");
    
    auto zoneId = NAPA_STRING_REF_TO_STD_STRING(id);
    auto zone = ZoneImpl::Get(zoneId);
    if (!zone) {
        return nullptr;
    }

    return new napa_zone { std::move(zoneId), std::move(zone) };
}

napa_zone_handle napa_zone_get_current() {
    NAPA_ASSERT(_initialized, "Napa wasn't initialized");

    auto zone = reinterpret_cast<ZoneImpl*>(napa::module::WorkerContext::Get(napa::module::WorkerContextItem::ZONE));
    if (zone == nullptr) {
        LOG_WARNING("Api", "Trying to get current zone from a thread that is not associated with a zone");
        return nullptr;
    }

    return napa_zone_get(STD_STRING_TO_NAPA_STRING_REF(zone->GetId()));
}

napa_response_code napa_zone_init(napa_zone_handle handle, napa_string_ref settings) {
    NAPA_ASSERT(_initialized, "Napa wasn't initialized");
    NAPA_ASSERT(handle, "Zone handle is null");

    // Zone settings are based on platform settings.
    ZoneSettings zoneSettings = _platformSettings;
    if (!napa::settings_parser::ParseFromString(NAPA_STRING_REF_TO_STD_STRING(settings), zoneSettings)) {
        LOG_ERROR("Api", "Failed to parse zone settings: %s", settings.data);
        return NAPA_RESPONSE_SETTINGS_PARSER_ERROR;
    }

    zoneSettings.id = handle->id;

    // Create the actual zone.
    handle->zone = ZoneImpl::Create(zoneSettings);
    if (handle->zone == nullptr) {
        LOG_ERROR("Api", "Failed to initialize zone: %s", handle->id.c_str());
        return NAPA_RESPONSE_ZONE_INIT_ERROR;
    }

    LOG_INFO("Api", "Napa zone '%s' initialized successfully", handle->id.c_str());

    return NAPA_RESPONSE_SUCCESS;
}

napa_response_code napa_zone_release(napa_zone_handle handle) {
    NAPA_ASSERT(_initialized, "Napa wasn't initialized");
    NAPA_ASSERT(handle, "Zone handle is null");

    handle->zone = nullptr;
    delete handle;

    return NAPA_RESPONSE_SUCCESS;
}

napa_string_ref napa_zone_get_id(napa_zone_handle handle) {
    NAPA_ASSERT(_initialized, "Napa wasn't initialized");
    NAPA_ASSERT(handle, "Zone handle is null");
    
    return STD_STRING_TO_NAPA_STRING_REF(handle->id);
}

void napa_zone_broadcast(napa_zone_handle handle,
                         napa_string_ref source,
                         napa_zone_broadcast_callback callback,
                         void* context) {
    NAPA_ASSERT(_initialized, "Napa wasn't initialized");
    NAPA_ASSERT(handle, "Zone handle is null");
    NAPA_ASSERT(handle->zone, "Zone handle wasn't initialized");

    handle->zone->Broadcast(NAPA_STRING_REF_TO_STD_STRING(source), [callback, context](napa_response_code code) {
        callback(code, context);
    });
}

void napa_zone_execute(napa_zone_handle handle,
                       napa_zone_request request,
                       napa_zone_execute_callback callback,
                       void* context) {
    NAPA_ASSERT(_initialized, "Napa wasn't initialized");
    NAPA_ASSERT(handle, "Zone handle is null");
    NAPA_ASSERT(handle->zone, "Zone handle wasn't initialized");

    ExecuteRequest req;
    req.module = request.module;
    req.function = request.function;
    
    req.arguments.reserve(request.arguments_count);
    for (size_t i = 0; i < request.arguments_count; i++) {
        req.arguments.emplace_back(request.arguments[i]);
    }

    req.timeout = request.timeout;
    
    // Assume ownership of transport context
    req.transportContext.reset(reinterpret_cast<napa::transport::TransportContext*>(request.transport_context));

    handle->zone->Execute(req, [callback, context](ExecuteResponse response) {
        napa_zone_response res;
        res.code = response.code;
        res.error_message = STD_STRING_TO_NAPA_STRING_REF(response.errorMessage);
        res.return_value = STD_STRING_TO_NAPA_STRING_REF(response.returnValue);
        
        // Release ownership of transport context
        res.transport_context = reinterpret_cast<void*>(response.transportContext.release());

        callback(res, context);
    });
}

static napa_response_code napa_initialize_common() {
    if (!napa::providers::Initialize(_platformSettings)) {
        return NAPA_RESPONSE_PROVIDERS_INIT_ERROR;
    }

    if (_platformSettings.initV8) {
        if (!napa::v8_common::Initialize()) {
            return NAPA_RESPONSE_V8_INIT_ERROR;
        }
    }

    _initialized = true;

    LOG_INFO("Api", "Napa initialized successfully");

    return NAPA_RESPONSE_SUCCESS;
}

napa_response_code napa_initialize(napa_string_ref settings) {
    NAPA_ASSERT(!_initialized, "Napa was already initialized");

    if (!napa::settings_parser::ParseFromString(NAPA_STRING_REF_TO_STD_STRING(settings), _platformSettings)) {
        return NAPA_RESPONSE_SETTINGS_PARSER_ERROR;
    }

    return napa_initialize_common();
}

napa_response_code napa_initialize_from_console(int argc, char* argv[]) {
    NAPA_ASSERT(!_initialized, "Napa was already initialized");

    if (!napa::settings_parser::ParseFromConsole(argc, argv, _platformSettings)) {
        return NAPA_RESPONSE_SETTINGS_PARSER_ERROR;
    }

    return napa_initialize_common();
}

napa_response_code napa_shutdown() {
    NAPA_ASSERT(_initialized, "Napa wasn't initialized");

    napa::providers::Shutdown();

    if (_platformSettings.initV8) {
        napa::v8_common::Shutdown();
    }

    LOG_INFO("Api", "Napa shutdown successfully");

    return NAPA_RESPONSE_SUCCESS;
}


#define NAPA_RESPONSE_CODE_DEF(symbol, string_rep) string_rep

static const char* NAPA_REPONSE_CODE_STRINGS[] = {
#include "napa/response-codes.inc"
};

#undef NAPA_RESPONSE_CODE_DEF

template<class T, size_t N>
constexpr size_t size(T(&)[N]) { return N; }

const char* napa_response_code_to_string(napa_response_code code) {
    NAPA_ASSERT(code < size(NAPA_REPONSE_CODE_STRINGS), "response code out of range");

    return NAPA_REPONSE_CODE_STRINGS[code];
}


///////////////////////////////////////////////////////////////
/// Implementation of napa.memory C API

void* napa_malloc(size_t size) {
    return ::malloc(size);
}

void napa_free(void* pointer, size_t /*size_hint*/) {
    ::free(pointer);
}

namespace {
    napa_allocate_callback _global_allocate = napa_malloc;
    napa_deallocate_callback _global_deallocate = napa_free;
} // namespace

void napa_allocator_set(
    napa_allocate_callback allocate_callback, 
    napa_deallocate_callback deallocate_callback) {
    
    NAPA_ASSERT(allocate_callback != nullptr, "'allocate_callback' should be a valid function.");
    NAPA_ASSERT(deallocate_callback != nullptr, "'deallocate_callback' should be a valid function.");

    _global_allocate = allocate_callback;
    _global_deallocate = deallocate_callback;
}

void* napa_allocate(size_t size) {
    return _global_allocate(size);
}

void napa_deallocate(void* pointer, size_t size_hint) {
    _global_deallocate(pointer, size_hint);
}
