// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <napa/capi.h>

#include <providers/providers.h>
#include <settings/settings-parser.h>
#include <utils/debug.h>
#include <v8/v8-common.h>
#include <zone/napa-zone.h>
#include <zone/node-zone.h>
#include <zone/worker-context.h>

#include <napa/log.h>

#include <atomic>
#include <fstream>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <vector>


using namespace napa;

static std::atomic<bool> _initialized(false);
static settings::PlatformSettings _platformSettings;

/// <summary> a simple wrapper around Zone for managing lifetime using shared_ptr. </summary>
struct napa_zone {
    std::string id;
    std::shared_ptr<zone::Zone> zone;
};

napa_zone_handle napa_zone_create(napa_string_ref id) {
    NAPA_ASSERT(_initialized, "Napa wasn't initialized");

    // The actual zone is created upon initialization.
    return new napa_zone { NAPA_STRING_REF_TO_STD_STRING(id), nullptr };
}

napa_zone_handle napa_zone_get(napa_string_ref id) {
    NAPA_ASSERT(_initialized, "Napa wasn't initialized");
    
    auto zoneId = NAPA_STRING_REF_TO_STD_STRING(id);
    std::shared_ptr<zone::Zone> zone;
    if (zoneId == "node") {
        zone = zone::NodeZone::Get();
    } else {
        zone = zone::NapaZone::Get(zoneId);
    }

    if (!zone) {
        return nullptr;
    }

    return new napa_zone { std::move(zoneId), std::move(zone) };
}

napa_zone_handle napa_zone_get_current() {
    NAPA_ASSERT(_initialized, "Napa platform wasn't initialized");

    auto zone = reinterpret_cast<zone::Zone*>(zone::WorkerContext::Get(zone::WorkerContextItem::ZONE));
    if (zone == nullptr) {
        LOG_WARNING("Api", "Trying to get current zone from a thread that is not associated with a zone");
        return nullptr;
    }

    return napa_zone_get(STD_STRING_TO_NAPA_STRING_REF(zone->GetId()));
}

napa_result_code napa_zone_init(napa_zone_handle handle, napa_string_ref settings) {
    NAPA_ASSERT(_initialized, "Napa platform wasn't initialized");
    NAPA_ASSERT(handle, "Zone handle is null");

    settings::ZoneSettings zoneSettings;
    if (!napa::settings::ParseFromString(NAPA_STRING_REF_TO_STD_STRING(settings), zoneSettings)) {
        NAPA_DEBUG("Api", "Failed to parse zone settings: %s", settings.data);
        return NAPA_RESULT_SETTINGS_PARSER_ERROR;
    }

    zoneSettings.id = handle->id;

    // Create the actual zone.
    handle->zone = zone::NapaZone::Create(zoneSettings);
    if (handle->zone == nullptr) {
        NAPA_DEBUG("Api", "Failed to create Napa zone '%s' with settings: %s", handle->id.c_str(), settings.data);
        return NAPA_RESULT_ZONE_INIT_ERROR;
    }

    NAPA_DEBUG("Api", "Napa zone '%s' created with settings: %s", handle->id.c_str(), settings.data);

    return NAPA_RESULT_SUCCESS;
}

napa_result_code napa_zone_release(napa_zone_handle handle) {
    NAPA_ASSERT(_initialized, "Napa platform wasn't initialized");
    NAPA_ASSERT(handle, "Zone handle is null");

    handle->zone = nullptr;
    delete handle;

    return NAPA_RESULT_SUCCESS;
}

napa_string_ref napa_zone_get_id(napa_zone_handle handle) {
    NAPA_ASSERT(_initialized, "Napa platform wasn't initialized");
    NAPA_ASSERT(handle, "Zone handle is null");
    
    return STD_STRING_TO_NAPA_STRING_REF(handle->id);
}

void napa_zone_broadcast(napa_zone_handle handle,
                         napa_string_ref source,
                         napa_zone_broadcast_callback callback,
                         void* context) {
    NAPA_ASSERT(_initialized, "Napa platform wasn't initialized");
    NAPA_ASSERT(handle, "Zone handle is null");
    NAPA_ASSERT(handle->zone, "Zone handle wasn't initialized");

    handle->zone->Broadcast(NAPA_STRING_REF_TO_STD_STRING(source), [callback, context](napa_result_code code) {
        callback(code, context);
    });
}

void napa_zone_execute(napa_zone_handle handle,
                       napa_zone_function_spec spec,
                       napa_zone_execute_callback callback,
                       void* context) {
    NAPA_ASSERT(_initialized, "Napa platform wasn't initialized");
    NAPA_ASSERT(handle, "Zone handle is null");
    NAPA_ASSERT(handle->zone, "Zone handle wasn't initialized");

    FunctionSpec req;
    req.module = spec.module;
    req.function = spec.function;
    
    req.arguments.reserve(spec.arguments_count);
    for (size_t i = 0; i < spec.arguments_count; i++) {
        req.arguments.emplace_back(spec.arguments[i]);
    }

    req.options = spec.options;
    
    // Assume ownership of transport context
    req.transportContext.reset(reinterpret_cast<napa::transport::TransportContext*>(spec.transport_context));

    handle->zone->Execute(req, [callback, context](Result result) {
        napa_zone_result res;
        res.code = result.code;
        res.error_message = STD_STRING_TO_NAPA_STRING_REF(result.errorMessage);
        res.return_value = STD_STRING_TO_NAPA_STRING_REF(result.returnValue);
        
        // Release ownership of transport context
        res.transport_context = reinterpret_cast<void*>(result.transportContext.release());

        callback(res, context);
    });
}

static napa_result_code napa_initialize_common() {
    if (!napa::providers::Initialize(_platformSettings)) {
        return NAPA_RESULT_PROVIDERS_INIT_ERROR;
    }

    if (!napa::v8_common::Initialize()) {
        return NAPA_RESULT_V8_INIT_ERROR;
    }

    _initialized = true;

    NAPA_DEBUG("Api", "Napa platform initialized successfully");

    return NAPA_RESULT_SUCCESS;
}

napa_result_code napa_initialize(napa_string_ref settings) {
    NAPA_ASSERT(!_initialized, "Napa platform was already initialized");

    if (!napa::settings::ParseFromString(NAPA_STRING_REF_TO_STD_STRING(settings), _platformSettings)) {
        return NAPA_RESULT_SETTINGS_PARSER_ERROR;
    }

    return napa_initialize_common();
}

napa_result_code napa_initialize_from_console(int argc, char* argv[]) {
    NAPA_ASSERT(!_initialized, "Napa platform was already initialized");

    if (!napa::settings::ParseFromConsole(argc, argv, _platformSettings)) {
        return NAPA_RESULT_SETTINGS_PARSER_ERROR;
    }

    return napa_initialize_common();
}

napa_result_code napa_shutdown() {
    NAPA_ASSERT(_initialized, "Napa platform wasn't initialized");

    napa::providers::Shutdown();
    napa::v8_common::Shutdown();

    LOG_INFO("Api", "Napa platform shutdown successfully");

    return NAPA_RESULT_SUCCESS;
}


#define NAPA_RESULT_CODE_DEF(symbol, string_rep) string_rep

static const char* NAPA_REPONSE_CODE_STRINGS[] = {
#include "napa/result-codes.inc"
};

#undef NAPA_RESULT_CODE_DEF

template<class T, size_t N>
constexpr size_t size(T(&)[N]) { return N; }

const char* napa_result_code_to_string(napa_result_code code) {
    NAPA_ASSERT(code < size(NAPA_REPONSE_CODE_STRINGS), "result code out of range");

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
