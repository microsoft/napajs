#include "napa-runtime-c.h"

#include "container.h"
#include "providers/providers.h"
#include "settings/settings-parser.h"

#include <thread>

#include <iostream>

using namespace napa::runtime;

static internal::Settings _globalSettings;

napa_container_handle napa_container_create() {
    std::cout << "napa_container_create()" << std::endl;

    return reinterpret_cast<napa_container_handle>(new internal::Container());
}

napa_response_code napa_container_init(napa_container_handle handle, napa_string_ref settings) {
    // Container settings are based on global settings.
    auto containerSettings = std::make_unique<internal::Settings>(_globalSettings);

    if (internal::settings_parser::ParseFromString(NAPA_STRING_REF_TO_STD_STRING(settings), *containerSettings)) {
        return NAPA_RESPONSE_SETTINGS_PARSER_ERROR;
    }

    auto container = reinterpret_cast<internal::Container*>(handle);

    container->Initialize(std::move(containerSettings));

    return NAPA_RESPONSE_SUCCESS;
}

napa_response_code napa_container_set_global_value(napa_container_handle handle ,napa_string_ref key, void* value) {
    std::cout << "napa_container_set_global_value()" << std::endl;
    std::cout << "\tkey: " << key.data << std::endl;

    return NAPA_RESPONSE_SUCCESS;
}

void napa_container_load_file(napa_container_handle handle,
                              napa_string_ref file,
                              napa_container_load_callback callback,
                              void* context) {
    std::cout << "napa_container_load_file()" << std::endl;
    std::cout << "\tfile: " << file.data << std::endl;

    // Mock async response
    std::thread([callback, context]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));

        callback(NAPA_RESPONSE_SUCCESS, context);
    }).detach();
}

napa_response_code napa_container_load_file_sync(napa_container_handle handle, napa_string_ref file) {
    std::cout << "napa_container_load_file_sync()" << std::endl;
    std::cout << "\tfile: " << file.data << std::endl;

    return NAPA_RESPONSE_SUCCESS;
}

void napa_container_load(napa_container_handle handle,
                         napa_string_ref source,
                         napa_container_load_callback callback,
                         void* context) {
    std::cout << "napa_container_load()" << std::endl;
    std::cout << "\tsource: " << source.data << std::endl;

    // Mock async response
    std::thread([callback, context]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));

        callback(NAPA_RESPONSE_SUCCESS, context);
    }).detach();
}

napa_response_code napa_container_load_sync(napa_container_handle handle, napa_string_ref source) {
    std::cout << "napa_container_load_sync()" << std::endl;
    std::cout << "\tsource: " << source.data << std::endl;

    return NAPA_RESPONSE_SUCCESS;
}

void napa_container_run(napa_container_handle handle,
                        napa_string_ref func,
                        size_t argc,
                        const napa_string_ref argv[],
                        napa_container_run_callback callback,
                        void* context,
                        uint32_t timeout) {
    std::cout << "napa_container_run()" << std::endl;
    std::cout << "\tfunc: " << func.data << std::endl;
    std::cout << "\targc: " << argc << std::endl;

    for (int i = 0; i < argc; i++) {
        std::cout << "\t\t[" << i << "] " << argv[i].data << std::endl;
    }
    std::cout << "\ttimeout: " << timeout << std::endl;

    // Mock async response
    std::thread([callback, context]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));

        napa_container_response response;

        response.code = NAPA_RESPONSE_SUCCESS;
        response.error_message = CREATE_NAPA_STRING_REF("");
        response.return_value = CREATE_NAPA_STRING_REF("{\"score\":2412}");

        callback(response, context);
    }).detach();
}

napa_container_response napa_container_run_sync(napa_container_handle handle,
                                                napa_string_ref func,
                                                size_t argc,
                                                const napa_string_ref argv[],
                                                uint32_t timeout) {
    std::cout << "napa_container_run_sync()" << std::endl;
    std::cout << "\tfunc: " << func.data << std::endl;
    std::cout << "\targc: " << argc << std::endl;
    
    for (int i = 0; i < argc; i++) {
        std::cout << "\t\t[" << i << "] " << argv[i].data << std::endl;
    }
    std::cout << "\ttimeout: " << timeout << std::endl;

    return napa_container_response {
        NAPA_RESPONSE_SUCCESS,
        CREATE_NAPA_STRING_REF(""),
        CREATE_NAPA_STRING_REF("{\"score\":2412}")
    };
}

napa_response_code napa_container_release(napa_container_handle handle) {
    return NAPA_RESPONSE_SUCCESS;
}

napa_response_code napa_initialize(napa_string_ref settings) {
    if (!internal::settings_parser::ParseFromString(NAPA_STRING_REF_TO_STD_STRING(settings), _globalSettings)) {
        return NAPA_RESPONSE_SETTINGS_PARSER_ERROR;
    }

    if (!napa::providers::Initialize(_globalSettings)) {
        return NAPA_RESPONSE_PROVIDERS_INIT_ERROR;
    }

    return NAPA_RESPONSE_SUCCESS;
}

napa_response_code napa_initialize_from_console(int argc, char* argv[]) {
    if (!internal::settings_parser::ParseFromConsole(argc, argv, _globalSettings)) {
        return NAPA_RESPONSE_SETTINGS_PARSER_ERROR;
    }

    if (!napa::providers::Initialize(_globalSettings)) {
        return NAPA_RESPONSE_PROVIDERS_INIT_ERROR;
    }

    return NAPA_RESPONSE_SUCCESS;
}

napa_response_code napa_shutdown() {
    napa::providers::Shutdown();

    return NAPA_RESPONSE_SUCCESS;
}


#define NAPA_RESPONSE_CODE_DEF(symbol, string_rep) string_rep

static const char* NAPA_REPONSE_CODE_STRINGS[] = {
#include "napa/response-codes.inc"
};

#undef NAPA_RESPONSE_CODE_DEF


const char* napa_response_code_to_string(napa_response_code code) {
    std::cout << "napa_shutdownnapa_response_code_to_string()" << std::endl;

    // TODO: assert code is in array boundaries

    return NAPA_REPONSE_CODE_STRINGS[code];
}
