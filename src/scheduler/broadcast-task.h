#pragma once

#include "task.h"

#include "napa/common.h"

#include <functional>
#include <string>

namespace napa {
namespace scheduler {

    /// <summary> A task for loading javascript source code. </summary>
    class BroadcastTask : public Task {
    public:
        
        /// <summary> Signature of the callback function. </summary>
        typedef std::function<void(NapaResponseCode responseCode)> BroadcastTaskCallback;

        /// <summary> Constructor. </summary>
        /// <param name="source"> The JS source code to load on the isolate the runs this task. </param>
        /// <param name="sourceOrigin"> The origin of the source code. </param>
        /// <param name="callback"> A callback that is triggered when the task execution completed. </param>
        BroadcastTask(std::string source,
            std::string sourceOrigin = "",
            BroadcastTaskCallback callback = [](NapaResponseCode) {});

        /// <summary> Overrides Task.Execute to define loading execution logic. </summary>
        virtual void Execute() override;

    private:
        std::string _source;
        std::string _sourceOrigin;
        BroadcastTaskCallback _callback;
    };
}
}
