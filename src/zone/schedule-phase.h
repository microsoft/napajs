// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <stdint.h>

namespace napa {
namespace zone {

    // Define the phase (like priority related type) of tasks. 
    // To be used mainly by schduler and worker to schedule its tasks.
    enum class SchedulePhase : uint32_t {
        DefaultPhase = 0,
        ImmediatePhase = 1
    };
    
};
}
