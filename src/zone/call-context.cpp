// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

// See: https://groups.google.com/forum/#!topic/nodejs/onA0S01INtw
#include <node.h>

#include "call-context.h"

#include <napa/log.h>
#include <napa/v8-helpers.h>

#include <stdint.h>

using namespace napa::zone;

CallContext::CallContext(const napa::FunctionSpec& spec, napa::ExecuteCallback callback) : 
    _module(NAPA_STRING_REF_TO_STD_STRING(spec.module)),
    _function(NAPA_STRING_REF_TO_STD_STRING(spec.function)),
    _callback(callback),
    _finished(false) {

    // Audit start time.
    _startTime = std::chrono::high_resolution_clock::now();
    
    _arguments.reserve(spec.arguments.size());
    for (auto& arg : spec.arguments) {
        _arguments.emplace_back(NAPA_STRING_REF_TO_STD_STRING(arg));
    }
    _options = spec.options;

    // Pass ownership of the transport context.
    _transportContext = std::move(spec.transportContext);
}

bool CallContext::Resolve(std::string marshalledResult) {
    auto expected = false;
    if (!_finished.compare_exchange_strong(expected, true)) {
        return false;
    }

    NAPA_DEBUG("CallTask", "Call to \"%s.%s\" is resolved successfully.", _module.c_str(), _function.c_str());

    _callback({ 
        NAPA_RESULT_SUCCESS, 
        "", 
        std::move(marshalledResult),
        std::move(_transportContext) 
    });
    return true;
}

bool CallContext::Reject(napa::ResultCode code, std::string reason) {
    auto expected = false;
    if (!_finished.compare_exchange_strong(expected, true)) {
        return false;
    }

    NAPA_DEBUG("CallTask", "Call to \"%s.%s\" was rejected: %s.", _module.c_str(), _function.c_str(), reason.c_str());

    _callback({ code, reason, "", std::move(_transportContext) });
    return true;
}

bool CallContext::IsFinished() const {
    return _finished;
}

const std::string& CallContext::GetModule() const {
    return _module;
}

const std::string& CallContext::GetFunction() const {
    return _function;
}

const std::vector<std::string>& CallContext::GetArguments() const {
    return _arguments;
}

napa::transport::TransportContext& CallContext::GetTransportContext() {
    return *_transportContext.get();
}

const napa::CallOptions& CallContext::GetOptions() const {
    return _options;
}

std::chrono::nanoseconds CallContext::GetElapse() const {
    return std::chrono::high_resolution_clock::now() - _startTime;
}