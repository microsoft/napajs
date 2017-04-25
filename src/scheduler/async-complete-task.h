#pragma once

#include "async-context.h"

#include <scheduler/task.h>

#include <memory>

namespace napa {
namespace module {

    /// <summary> A task to run Javascript callback after asynchronous callback completes. </summary>
    class AsyncCompleteTask : public scheduler::Task {
    public:

        /// <summary> Constructor. </summary>
        /// <param name="context"> Structure containing asynchronous work's context. </param>
        AsyncCompleteTask(std::shared_ptr<AsyncContext> context);

        /// <summary> Overrides Task.Execute to define running execution logic. </summary>
        virtual void Execute() override;

    private:

        std::shared_ptr<AsyncContext> _context;
    };

}
}