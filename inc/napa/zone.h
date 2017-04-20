#pragma once

#include "napa/common.h"

#include <functional>
#include <future>
#include <memory>
#include <string>
#include <vector>


namespace napa {

    // TODO @asib: remove this when the actual TransportContext is added to napa::memory
    namespace memory {
        class TransportContext {};
    }

    /// <summary> Represents an execution request. </summary>
    struct ExecuteRequest {

        /// <summary> The module that exports the function to execute. </summary>
        NapaStringRef module = EMPTY_NAPA_STRING_REF;

        /// <summary> The function to execute. </summary>
        NapaStringRef function = EMPTY_NAPA_STRING_REF;

        /// <summary> The function arguments. </summary>
        std::vector<NapaStringRef> arguments;

        /// <summary> Timeout in milliseconds - Use 0 for inifinite. </summary>
        uint32_t timeout = 0;

        /// <summary> Used for transporting shared_ptr and unique_ptr across zones/workers. </summary>
        mutable std::unique_ptr<napa::memory::TransportContext> transportContext;
    };

    /// <summary> Represents an execution response. </summary>
    struct ExecuteResponse {

        /// <summary> A response code. </summary>
        NapaResponseCode code;

        /// <summary> The error message in case of an error. </summary>
        std::string errorMessage;

        /// <summary> The return value in case of success. </summary>
        std::string returnValue;

        /// <summary> Used for transporting shared_ptr and unique_ptr across zones/workers. </summary>
        mutable std::unique_ptr<napa::memory::TransportContext> transportContext;
    };

    /// <summary> Callback signature. </summary>
    typedef std::function<void(NapaResponseCode)> BroadcastCallback;
    typedef std::function<void(ExecuteResponse)> ExecuteCallback;

    /// <summary> Base class for napa zone. </summary>
    struct Zone {

        /// <summary> Get the zone id. </summary>
        virtual const std::string& GetId() const = 0;

        /// <summary> Compiles and run the provided source code on all zone workers asynchronously. </summary>
        /// <param name="source"> The source code. </param>
        /// <param name="callback"> A callback that is triggered when broadcasting is done. </param>
        virtual void Broadcast(const std::string& source, BroadcastCallback callback) = 0;

        /// <summary> Executes a pre-loaded JS function asynchronously. </summary>
        /// <param name="request"> The execution request. </param>
        /// <param name="callback"> A callback that is triggered when execution is done. </param>
        virtual void Execute(const ExecuteRequest& request, ExecuteCallback callback) = 0;

        /// <summary> Compiles and run the provided source code on all zone workers synchronously. </summary>
        /// <param name="source"> The source code. </param>
        virtual NapaResponseCode BroadcastSync(const std::string& source) {
            std::promise<NapaResponseCode> prom;
            auto fut = prom.get_future();

            Broadcast(source, [&prom](NapaResponseCode code) {
                prom.set_value(code);
            });

            return fut.get();
        }

        /// <summary> Executes a pre-loaded JS function synchronously. </summary>
        /// <param name="request"> The execution request. </param>
        virtual ExecuteResponse ExecuteSync(const ExecuteRequest& request) {
            std::promise<ExecuteResponse> prom;
            auto fut = prom.get_future();

            Execute(request, [&prom](ExecuteResponse response) {
                prom.set_value(std::move(response));
            });

            return fut.get();
        }

        /// <summary> Virtual destructor. </summary>
        virtual ~Zone() {}
    };
}