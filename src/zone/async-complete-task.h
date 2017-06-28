#pragma once

#include "async-context.h"

#include <zone/task.h>

#include <memory>

namespace napa {
namespace zone {

    /// <summary> A task to run Javascript callback after asynchronous callback completes. </summary>
    class AsyncCompleteTask : public Task {
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