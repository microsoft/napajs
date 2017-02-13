#pragma once

#include "task.h"

#include "napa/common.h"

#include <functional>
#include <string>
#include <vector>

namespace napa {
namespace scheduler {

    /// <summary> A task for running a pre-loaded javascript function. </summary>
    class RunTask : public Task {
    public:

        /// <summary> Signature of the callback function. </summary>
        typedef std::function<void(NapaResponseCode code,
                                   NapaStringRef errorMessage,
                                   NapaStringRef returnValue)> RunTaskCallback;

        /// <summary> Constructor. </summary>
        /// <param name="func"> The pre-load function name to run. </param>
        /// <param name="args"> The arguments to pass to the pre-loaded function. </param>
        /// <param name="callback"> A callback that is triggered when the task execution completed. </param>
        RunTask(std::string func, std::vector<std::string> args, RunTaskCallback callback);

        /// <summary> Overrides Task.Execute to define running execution logic. </summary>
        virtual void Execute() override;

    private:
        std::string _func;
        std::vector<std::string> _args;
        RunTaskCallback _callback;
    };
}
}
