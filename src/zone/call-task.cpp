// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

// See: https://groups.google.com/forum/#!topic/nodejs/onA0S01INtw
#ifdef BUILDING_NODE_EXTENSION
#include <node.h>
#endif

#include "call-task.h"

#include <module/core-modules/napa/call-context-wrap.h>
#include <utils/debug.h>

using namespace napa::zone;
using namespace napa::v8_helpers;

napa::zone::CallTask::CallTask(std::shared_ptr<CallContext> context) : 
    _context(std::move(context)) {
}

void CallTask::Execute() {
    NAPA_DEBUG("CallTask", "Begin executing function (%s.%s).", _context->GetModule().c_str(), _context->GetFunction().c_str());

    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    auto context = isolate->GetCurrentContext();

    // Get the module based main function from global scope.
    auto executeFunction = context->Global()->Get(MakeExternalV8String(isolate, "__napa_zone_call__"));
    JS_ENSURE(isolate, executeFunction->IsFunction(), "__napa_zone_call__ function must exist in global scope");

    // Create task wrap.
    auto contextWrap = napa::module::CallContextWrap::NewInstance(_context);
    v8::Local<v8::Value> argv[] = { contextWrap };

    // Execute the function.
    v8::TryCatch tryCatch(isolate);
    auto res = v8::Local<v8::Function>::Cast(executeFunction)->Call(
        isolate->GetCurrentContext(),
        context->Global(),
        1,
        argv);

    // Terminating an isolate may occur from a different thread, i.e. from timeout service.
    // If the function call already finished successfully when the isolate is terminated it may lead
    // to one the following:
    //      1. Terminate was called before tryCatch.HasTerminated(), the user gets an error code.
    //      2. Terminate was called after tryCatch.HasTerminated(), the user gets a success code.
    //
    //  In both cases the isolate is being restored since this happens before each task executes.
    if (tryCatch.HasTerminated()) {
        if (_terminationReason == TerminationReason::TIMEOUT) {
            (void)_context->Reject(NAPA_RESULT_TIMEOUT, "Terminated due to timeout");
        } else {
            (void)_context->Reject(NAPA_RESULT_INTERNAL_ERROR, "Terminated with unknown reason");
        }
        return;
    }

    NAPA_ASSERT(!tryCatch.HasCaught(), "__napa_zone_call__ should catch all user exceptions and reject task.");
}
