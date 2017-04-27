#pragma once

#include "terminable-task.h"

#include <napa/common.h>
#include <napa/zone.h>

#include <functional>
#include <string>
#include <vector>

namespace napa {
namespace scheduler {

    /// <summary> A task for executing pre-loaded javascript functions. </summary>
    class ExecuteTask : public TerminableTask {
    public:

        /// <summary> Constructor. </summary>
        /// <param name="request"> The execution request. </param>
        /// <param name="callback"> A callback that is triggered when the execute task is completed. </param>
        ExecuteTask(const ExecuteRequest& request, ExecuteCallback callback);

        /// <summary> Overrides Task.Execute to define execution logic. </summary>
        virtual void Execute() override;

    private:
        std::string _module;
        std::string _func;
        std::vector<std::string> _args;
        ExecuteCallback _callback;
        std::unique_ptr<napa::transport::TransportContext> _transportContext;
    };
}
}
