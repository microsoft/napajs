// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

// See: https://groups.google.com/forum/#!topic/nodejs/onA0S01INtw
#ifdef BUILDING_NODE_EXTENSION
#include <node.h>
#endif

#include "terminable-task.h"

#include <v8.h>

using namespace napa::zone;

void TerminableTask::Terminate(TerminationReason reason, v8::Isolate* isolate) {
    _terminationReason = reason;

    isolate->TerminateExecution();
}
