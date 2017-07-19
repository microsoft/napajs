// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "task.h"

namespace v8 {
    class Isolate;
}

namespace napa {
namespace zone {

    /// <summary> Specifies the possible reasons for termination. </summary>
    enum class TerminationReason {
        UNKNOWN,
        TIMEOUT
    };

    /// <summary> Base class for tasks that can be terminated. </summary>
    class TerminableTask : public Task {
    public:

        /// <summary> Use this to terminate a currently running task. </summary>
        /// <param name="reason"> The reason for the termination. </param>
        /// <param name="isolate"> The isolate this task currently runs on. </param>
        void Terminate(TerminationReason reason, v8::Isolate* isolate);

    protected:
        TerminationReason _terminationReason = TerminationReason::UNKNOWN;
    };
}
}
