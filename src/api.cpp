#include <napa-c.h>

#include "providers/providers.h"
#include "scheduler/load-task.h"
#include "scheduler/scheduler.h"
#include "scheduler/run-task.h"
#include "settings/settings-parser.h"
#include "v8/v8-common.h"

#include <napa-log.h>

#include <future>
#include <fstream>
#include <sstream>
#include <iostream>

#include <thread>


using namespace napa;
using namespace napa::scheduler;

static bool _initialized = false;
static Settings _globalSettings;

/// <summary> Simple representation of a container. Groups objects that are part of a single container. </summary>
struct Container {
    Settings settings;
    std::unique_ptr<Scheduler> scheduler;
};

napa_container_handle napa_container_create() {
    return reinterpret_cast<napa_container_handle>(new Container());
}

napa_response_code napa_container_init(napa_container_handle handle, napa_string_ref settings) {
    auto container = reinterpret_cast<Container*>(handle);

    // Container settings are based on global settings.
    container->settings = _globalSettings;

    if (napa::settings_parser::ParseFromString(NAPA_STRING_REF_TO_STD_STRING(settings), container->settings)) {
        return NAPA_RESPONSE_SETTINGS_PARSER_ERROR;
    }

    // Create the container's scheduler.
    container->scheduler = std::make_unique<Scheduler>(container->settings);

    return NAPA_RESPONSE_SUCCESS;
}

napa_response_code napa_container_set_global_value(napa_container_handle handle ,napa_string_ref key, void* value) {
    std::cout << "napa_container_set_global_value()" << std::endl;
    std::cout << "\tkey: " << key.data << std::endl;

    return NAPA_RESPONSE_SUCCESS;
}

static void napa_container_load_common(napa_container_handle handle,
                                       std::string source,
                                       LoadTask::LoadTaskCallback callback) {
    auto container = reinterpret_cast<Container*>(handle);

    auto loadTask = std::make_shared<LoadTask>(std::move(source), std::move(callback));

    // TODO @asib: wrap this task with a TimeoutTaskDecorator with a constant timeout.

    container->scheduler->ScheduleOnAllCores(std::move(loadTask));
}

static std::string read_file_content(napa_string_ref file) {
    std::ifstream ifs;
    ifs.open(NAPA_STRING_REF_TO_STD_STRING(file));

    if (!ifs.is_open()) {
        LOG_ERROR("Api", "Failed to open file %s", file.data);
        return "";
    }

    std::stringstream buffer;
    buffer << ifs.rdbuf();

    return buffer.str();
}

void napa_container_load_file(napa_container_handle handle,
                              napa_string_ref file,
                              napa_container_load_callback callback,
                              void* context) {
    // Although this is an async call, the reading of the file happens synchronously on the caller thread.
    // This is because the load task is distributed to all cores and we don't want to read the file multiple times.
    // We can spawn/reuse an additional thread just for reading before we schedule the load task, need to consider
    // if this becomes a bottleneck.
    auto fileContent = read_file_content(file);
    if (fileContent.empty()) {
        callback(NAPA_RESPONSE_LOAD_FILE_ERROR, context);
        return;
    }

    napa_container_load_common(handle, std::move(fileContent), [callback, context](napa_response_code code) {
        callback(code, context);
    });
}

napa_response_code napa_container_load_file_sync(napa_container_handle handle, napa_string_ref file) {
    auto fileContent = read_file_content(file);
    if (fileContent.empty()) {
        return NAPA_RESPONSE_LOAD_FILE_ERROR;
    }

    std::promise<napa_response_code> prom;
    auto fut = prom.get_future();
    napa_container_load_common(handle, std::move(fileContent), [&prom](napa_response_code code) {
        prom.set_value(code);
    });

    return fut.get();
}

void napa_container_load(napa_container_handle handle,
                         napa_string_ref source,
                         napa_container_load_callback callback,
                         void* context) {
    napa_container_load_common(handle,
                               NAPA_STRING_REF_TO_STD_STRING(source),
                               [callback, context](napa_response_code code) {
        callback(code, context);
    });
}

napa_response_code napa_container_load_sync(napa_container_handle handle, napa_string_ref source) {
    std::promise<napa_response_code> prom;
    auto fut = prom.get_future();
    napa_container_load_common(handle, NAPA_STRING_REF_TO_STD_STRING(source), [&prom](napa_response_code code) {
        prom.set_value(code);
    });

    return fut.get();
}

void napa_container_run_common(napa_container_handle handle,
                               napa_string_ref func,
                               size_t argc,
                               const napa_string_ref argv[],
                               RunTask::RunTaskCallback callback,
                               uint32_t timeout) {
    auto container = reinterpret_cast<Container*>(handle);

    // TODO @asib: Wrap run task with a timeout decorator.

    std::vector<std::string> args;
    args.reserve(argc);

    for (size_t i = 0; i < argc; i++) {
        args.emplace_back(NAPA_STRING_REF_TO_STD_STRING(argv[i]));
    }

    auto task = std::make_shared<RunTask>(NAPA_STRING_REF_TO_STD_STRING(func), std::move(args), std::move(callback));

    container->scheduler->Schedule(std::move(task));
}

void napa_container_run(napa_container_handle handle,
                        napa_string_ref func,
                        size_t argc,
                        const napa_string_ref argv[],
                        napa_container_run_callback callback,
                        void* context,
                        uint32_t timeout) {
    napa_container_run_common(handle, func, argc, argv,
        [callback, context](napa_response_code code, napa_string_ref errorMessage, napa_string_ref returnValue) {
        callback(napa_container_response{ code, errorMessage, returnValue }, context);
    }, timeout);
}

napa_container_response napa_container_run_sync(napa_container_handle handle,
                                                napa_string_ref func,
                                                size_t argc,
                                                const napa_string_ref argv[],
                                                uint32_t timeout) {
    std::promise<napa_container_response> prom;
    auto fut = prom.get_future();

    napa_container_run_common(handle, func, argc, argv,
        [&prom](napa_response_code code, napa_string_ref errorMessage, napa_string_ref returnValue) {
        prom.set_value(napa_container_response{ code, errorMessage, returnValue });
    }, timeout);

    return fut.get();
}

napa_response_code napa_container_release(napa_container_handle handle) {
    auto container = reinterpret_cast<Container*>(handle);

    delete container;

    return NAPA_RESPONSE_SUCCESS;
}

static napa_response_code napa_initialize_common() {
    if (!napa::providers::Initialize(_globalSettings)) {
        return NAPA_RESPONSE_PROVIDERS_INIT_ERROR;
    }

    if (_globalSettings.initV8) {
        napa::v8_common::Initialize();
    }

    _initialized = true;

    return NAPA_RESPONSE_SUCCESS;
}

napa_response_code napa_initialize(napa_string_ref settings) {
    assert(_initialized == false);

    if (!napa::settings_parser::ParseFromString(NAPA_STRING_REF_TO_STD_STRING(settings), _globalSettings)) {
        return NAPA_RESPONSE_SETTINGS_PARSER_ERROR;
    }

    return napa_initialize_common();
}

napa_response_code napa_initialize_from_console(int argc, char* argv[]) {
    assert(_initialized == false);

    if (!napa::settings_parser::ParseFromConsole(argc, argv, _globalSettings)) {
        return NAPA_RESPONSE_SETTINGS_PARSER_ERROR;
    }

    return napa_initialize_common();
}

napa_response_code napa_shutdown() {
    assert(_initialized == true);

    napa::providers::Shutdown();

    if (_globalSettings.initV8) {
        napa::v8_common::Shutdown();
    }

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
    assert(code < size(NAPA_REPONSE_CODE_STRINGS));

    return NAPA_REPONSE_CODE_STRINGS[code];
}
