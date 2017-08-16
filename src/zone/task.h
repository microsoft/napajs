// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

namespace napa {
namespace zone {

    /// <summary> Represents an execution logic that can be scheduled using the Napa scheduler. </summary>
    class Task {
    public:
        /// <summary> Executes the task. </summary>
        virtual void Execute() = 0;

        /// <summary> Virtual destructor. </summary>
        virtual ~Task() = default;
    };
}
}