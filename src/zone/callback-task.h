// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/exports.h>

#include <memory>
#include <functional>

#include <zone/task.h>

namespace napa {
namespace zone {

    /// <summary> A task to run a C++ callback task without cross isolation. </summary>
    class NAPA_API CallbackTask : public Task {
    public:
        typedef std::function<void(void)> Callback;    

        /// <summary> Constructor. </summary>
        /// <param name="context"> Structure containing asynchronous work's context. </param>
        CallbackTask(Callback callback);

        /// <summary> Overrides Task.Execute to define running execution logic. </summary>
        virtual void Execute();

    private:
        Callback _callback;
    };

}
}