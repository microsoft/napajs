// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "task.h"

#include "napa/types.h"

#include <functional>
#include <string>

namespace napa {
namespace zone {

    /// <summary> A task for evaluating javascript source code. </summary>
    class EvalTask : public Task {
    public:
        /// <summary> Constructor. </summary>
        /// <param name="source"> The JS source code to load on the isolate the runs this task. </param>
        /// <param name="sourceOrigin"> The origin of the source code. </param>
        /// <param name="callback"> A callback that is triggered when the task execution completed. </param>
        EvalTask(std::string source,
            std::string sourceOrigin = "",
            BroadcastCallback callback = [](ResultCode) {});

        /// <summary> Overrides Task.Execute to define loading execution logic. </summary>
        virtual void Execute() override;

    private:
        std::string _source;
        std::string _sourceOrigin;
        BroadcastCallback _callback;
    };
} // namespace zone
} // namespace napa
