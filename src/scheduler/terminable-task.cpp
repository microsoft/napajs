#include "terminable-task.h"

#include <v8.h>

using namespace napa::scheduler;

void TerminableTask::Terminate(TerminationReason reason, v8::Isolate* isolate) {
    _terminationReason = reason;

    isolate->TerminateExecution();
}
