// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "call-context.h"
#include "terminable-task.h"

#include <memory>

namespace napa {
namespace zone {

    /// <summary> A task for executing pre-loaded javascript functions. </summary>
    class CallTask : public TerminableTask {
    public:
        /// <summary> Constructor. </summary>
        /// <param name="context"> Call context. </param>
        CallTask(std::shared_ptr<CallContext> context);

        /// <summary> Overrides Task.Execute to define execution logic. </summary>
        virtual void Execute() override;

    private:
        /// <summary> Call context. </summary>
        std::shared_ptr<CallContext> _context;
    };
}
}
