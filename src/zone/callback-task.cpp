#include "callback-task.h"

using napa::zone::CallbackTask;

CallbackTask::CallbackTask(Callback callback) 
    : _callback(std::move(callback)) 
{ 
}

void CallbackTask::Execute() 
{
    _callback(); 
}

