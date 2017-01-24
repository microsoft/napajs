#include "core.h"

// Disable third party library warnnings
#pragma warning(push)
#pragma warning(disable: 4127)
#include <moodycamel/blockingconcurrentqueue.h>
#pragma warning(pop)

#include <v8.h>

#include <thread>

using namespace napa::runtime::internal;


struct Core::Impl {

    /// <summary> The core id. </summary>
    CoreId id;

    /// <summary> The thread that executes the tasks. </summary>
    std::thread coreThread;

    /// <summary> Queue for tasks scheduled on this core. </summary>
    moodycamel::BlockingConcurrentQueue<std::shared_ptr<Task>> tasks;

    /// <summary> V8 isolate associated with this core. </summary>
    v8::Isolate* isolate;

    /// <summary> A callback function that is called when a core becomes idle. </summary>
    std::function<void(CoreId)> idleNotficationCallback;
};

Core::Core(CoreId id, const Settings& settings) {
    // TODO @asib: add implementation
}

Core::~Core() {

}

void Core::Schedule(std::shared_ptr<Task> task) {
    // TODO @asib: add implementation
}

void Core::SubscribeForIdleNotifications(std::function<void(CoreId)> callback) {
    // TODO @asib: add implementation
}
