#pragma once

#include "task.h"

#include "napa/common.h"

#include <functional>
#include <string>

namespace napa {
namespace scheduler {

    /// <summary> A task for loading javascript source code. </summary>
    class LoadTask : public Task {
    public:
        
        /// <summary> Signature of the callback function. </summary>
        typedef std::function<void(NapaResponseCode responseCode)> LoadTaskCallback;

        /// <summary> Constructor. </summary>
        /// <param name="source"> The JS source code to load on the isolate the runs this task. </param>
        /// <param name="callback"> A callback that is triggered when the task execution completed. </param>
        LoadTask(std::string source, LoadTaskCallback callback = [](NapaResponseCode) {});

        /// <summary> Overrides Task.Execute to define loading execution logic. </summary>
        virtual void Execute() override;

    private:
        std::string _source;
        LoadTaskCallback _callback;
    };
}
}
