// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "napa/capi.h"

#include <functional>
#include <future>

namespace napa {

    /// <summary> Initializes napa with global scope settings. </summary>
    inline ResultCode Initialize(const std::string& settings = "") {
        return napa_initialize(STD_STRING_TO_NAPA_STRING_REF(settings));
    }

    /// <summary> Initialize napa using console provided arguments. </summary>
    inline ResultCode InitializeFromConsole(int argc, const char* argv[]) {
        return napa_initialize_from_console(argc, argv);
    }

    /// <summary> Shut down napa. </summary>
    inline ResultCode Shutdown() {
        return napa_shutdown();
    }

    /// <summary> C++ proxy around napa Zone C APIs. </summary>
    class Zone {
    public:

        /// <summary> Creates a new zone and wraps it with a zone proxy instance. </summary>
        /// <param name="id"> A unique id for the zone. </param>
        /// <param name="settings"> A settings string to set zone specific settings. </param>
        explicit Zone(const std::string& id, const std::string& settings = "") : _zoneId(id) {
            _handle = napa_zone_create(STD_STRING_TO_NAPA_STRING_REF(id));

            auto res = napa_zone_init(_handle, STD_STRING_TO_NAPA_STRING_REF(settings));
            if (res != NAPA_RESULT_SUCCESS) {
                napa_zone_release(_handle);
                throw std::runtime_error(napa_result_code_to_string(res));
            }
        }

        /// <summary> Releases the underlying zone handle. </summary>
        ~Zone() {
            napa_zone_release(_handle);
        }

        /// <summary> Compiles and run the provided source code on all zone workers asynchronously. </summary>
        /// <param name="source"> The source code. </param>
        /// <param name="callback"> A callback that is triggered when broadcasting is done. </param>
        const std::string& GetId() const {
            return _zoneId;
        }

        /// <see cref="Zone::Broadcast" />
        void Broadcast(const FunctionSpec& spec, BroadcastCallback callback) {
            // Will be deleted on when the callback scope ends.
            auto context = new BroadcastCallback(std::move(callback));

            napa_zone_function_spec req;
            req.module = spec.module;
            req.function = spec.function;
            req.arguments = spec.arguments.data();
            req.arguments_count = spec.arguments.size();
            req.options = spec.options;

            // Release ownership of transport context
            req.transport_context = reinterpret_cast<void*>(spec.transportContext.release());

            napa_zone_broadcast(_handle, req, [](napa_zone_result result, void* context) {
                // Ensures the context is deleted when this scope ends.
                std::unique_ptr<BroadcastCallback> callback(reinterpret_cast<BroadcastCallback*>(context));

                Result res;
                res.code = result.code;
                res.errorMessage = NAPA_STRING_REF_TO_STD_STRING(result.error_message);
                res.returnValue = NAPA_STRING_REF_TO_STD_STRING(result.return_value);

                // Assume ownership of transport context
                res.transportContext.reset(
                    reinterpret_cast<napa::transport::TransportContext*>(result.transport_context));

                (*callback)(std::move(res));
            }, context);
        }

        /// <summary> Executes a pre-loaded JS function synchronously. </summary>
        /// <param name="spec"> A function spec to call. </param>
        Result BroadcastSync(const FunctionSpec& spec) {
            if (GetCurrent()->GetId() == GetId()) {
                return {NAPA_RESULT_ZONE_BROADCAST_ERROR, "Cannot call `broadcastSync` on current zone.", "", nullptr};
            }

            std::promise<Result> prom;
            auto fut = prom.get_future();

            Broadcast(spec, [&prom](Result result) {
                prom.set_value(std::move(result));
            });

            return fut.get();
        }

        /// <summary> Executes a pre-loaded JS function asynchronously. </summary>
        /// <param name="spec"> A function spec to call. </param>
        /// <param name="callback"> A callback that is triggered when execution is done. </param>
        void Execute(const FunctionSpec& spec, ExecuteCallback callback) {
            // Will be deleted on when the callback scope ends.
            auto context = new ExecuteCallback(std::move(callback));

            napa_zone_function_spec req;
            req.module = spec.module;
            req.function = spec.function;
            req.arguments = spec.arguments.data();
            req.arguments_count = spec.arguments.size();
            req.options = spec.options;

            // Release ownership of transport context
            req.transport_context = reinterpret_cast<void*>(spec.transportContext.release());

            napa_zone_execute(_handle, req, [](napa_zone_result result, void* context) {
                // Ensures the context is deleted when this scope ends.
                std::unique_ptr<ExecuteCallback> callback(reinterpret_cast<ExecuteCallback*>(context));

                Result res;
                res.code = result.code;
                res.errorMessage = NAPA_STRING_REF_TO_STD_STRING(result.error_message);
                res.returnValue = NAPA_STRING_REF_TO_STD_STRING(result.return_value);

                // Assume ownership of transport context
                res.transportContext.reset(
                    reinterpret_cast<napa::transport::TransportContext*>(result.transport_context));

                (*callback)(std::move(res));
            }, context);
        }

        /// <summary> Executes a pre-loaded JS function synchronously. </summary>
        /// <param name="spec"> The function spec to call. </param>
        Result ExecuteSync(const FunctionSpec& spec) {
            std::promise<Result> prom;
            auto fut = prom.get_future();

            Execute(spec, [&prom](Result result) {
                prom.set_value(std::move(result));
            });

            return fut.get();
        }

        /// <summary> Recycle the zone </summary>
        void Recycle() {
            napa_zone_recycle(_handle);
        }

        /// <summary> Retrieves a new zone proxy for the zone id, throws if zone is not found. </summary>
        static std::unique_ptr<Zone> Get(const std::string& id) {
            auto handle = napa_zone_get(STD_STRING_TO_NAPA_STRING_REF(id));
            if (!handle) {
                throw std::runtime_error("No zone exists for id '" + id + "'");
            }

            return std::unique_ptr<Zone>(new Zone(id, handle));
        }

        /// <summary> Creates a proxy to the current zone, throws if non is associated with this thread. </summary>
        static std::unique_ptr<Zone> GetCurrent() {
            auto handle = napa_zone_get_current();
            if (!handle) {
                throw std::runtime_error("The calling thread is not associated with a zone");
            }

            auto zoneId = NAPA_STRING_REF_TO_STD_STRING(napa_zone_get_id(handle));
            return std::unique_ptr<Zone>(new Zone(std::move(zoneId), handle));
        }

    private:

        /// <summary> Private constructor to create a C++ zone proxy from a C handle. </summary>
        explicit Zone(const std::string& id, napa_zone_handle handle) : _zoneId(id), _handle(handle) {}

        /// <summary> The zone id. </summary>
        std::string _zoneId;

        /// <summary> Underlying zone handle. </summary>
        napa_zone_handle _handle;
    };
}