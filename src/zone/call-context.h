#pragma once

#include <napa/types.h>
#include <v8.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <vector>

namespace napa {
namespace zone {

    /// <summary> Context of calling a JavaScript function. </summary>
    class CallContext {

    public:
        /// <summary> Construct spec from external FunctionSpec. </summary>
        explicit CallContext(const napa::FunctionSpec& spec, napa::ExecuteCallback callback);

        /// <summary> Resolve current spec. </summary>
        /// <param name="result"> marshalled return value. </param>
        /// <returns> True if operation is successful, otherwise if task is already finished before. </returns>
        //bool Resolve(v8::Local<v8::Value> result);
        bool Resolve(std::string result);

        /// <summary> Reject current spec. </summary>
        /// <param name="resultCode"> Response code to return to user. </summary>
        /// <param name="reason"> Reason of cancellation. </summary>
        /// <returns> True if operation is successful, otherwise if task is already finished before. </returns>
        bool Reject(napa::ResultCode code, std::string reason);

        /// <summary> Returns whether current job is completed or cancelled. </summary>
        bool IsFinished() const;

        /// <summary> Get module name to load function. </summary>
        const std::string& GetModule() const;

        /// <summary> Get function name to execute. </summary>
        const std::string& GetFunction() const;

        /// <summary> Get marshalled arguments. </summary>
        const std::vector<std::string>& GetArguments() const;

        /// <summary> Get transport context. </summary>
        napa::transport::TransportContext& GetTransportContext();

        /// <summary> Get options. </summary>
        const napa::CallOptions& GetOptions() const;

        /// <summary> Get elapse since task start in nano-second. </summary>
        std::chrono::nanoseconds GetElapse() const;

    private:
        /// <summary> Module name. </summary>
        std::string _module;

        /// <summary> Function name. </summary>
        std::string _function;

        /// <summary> Arguments. </summary>
        std::vector<std::string> _arguments;

        /// <summary> Execute options. </summary>
        napa::CallOptions _options;

        /// <summary> Transport context. </summary>
        std::unique_ptr<napa::transport::TransportContext> _transportContext;

         /// <summary> Callback when task completes. </summary>
        napa::ExecuteCallback _callback;

        /// <summary> Whether this task is finished. </summary>
        std::atomic<bool> _finished;

        /// <summary> Call start time. </summary>
        std::chrono::high_resolution_clock::time_point _startTime;
    };
}
}