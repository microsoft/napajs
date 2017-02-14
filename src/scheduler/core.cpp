#include "core.h"

#include "v8/array-buffer-allocator.h"

// Disable third party library warnnings
#pragma warning(push)
#pragma warning(disable: 4127 4458 4068)
#include <moodycamel/blockingconcurrentqueue.h>
#pragma warning(pop)

#include <v8.h>

#include <stdlib.h>
#include <thread>

using namespace napa;
using namespace napa::scheduler;

// Forward declaration
static v8::Isolate* CreateIsolate(const Settings& settings);
static void ConfigureIsolate(v8::Isolate* isolate, const Settings& settings);

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

Core::Core(CoreId id,
           const Settings& settings,
           std::function<void(CoreId)> idleNotificationCallback)
           : _impl(std::make_unique<Core::Impl>()) {
    _impl->id = id;
    _impl->idleNotficationCallback = std::move(idleNotificationCallback);
    _impl->coreThread = std::thread(&Core::CoreThreadFunc, this, settings);
}

Core::~Core() {
    // Signal the thread loop that it should stop processing tasks.
    _impl->tasks.enqueue(nullptr);

    _impl->coreThread.join();

    if (_impl->isolate != nullptr) {
        _impl->isolate->Dispose();
    }
}


Core::Core(Core&&) = default;
Core& Core::operator=(Core&&) = default;

void Core::Schedule(std::shared_ptr<Task> task) {
    assert(task != nullptr);

    _impl->tasks.enqueue(std::move(task));
}

void Core::CoreThreadFunc(const Settings& settings) {
    _impl->isolate = CreateIsolate(settings);

    // If any user of the v8.dll uses a locker on any isolate, all isolates must be locked before use.
    // Since we are 1-1 with threads and isolates, a top level lock that is never released is ok.
    v8::Locker locker(_impl->isolate);

    ConfigureIsolate(_impl->isolate, settings);

    v8::Isolate::Scope isolateScope(_impl->isolate);
    v8::HandleScope handleScope(_impl->isolate);
    v8::Local<v8::Context> context = v8::Context::New(_impl->isolate);

    // We set an empty security token so callee can access caller's context.
    context->SetSecurityToken(v8::Undefined(_impl->isolate));
    v8::Context::Scope contextScope(context);

    // TODO @asib: This is where we need to call module loader for setting builtin and core modules.

    while (true) {
        std::shared_ptr<Task> task;
        
        // Retrieve a task to execute. Wait if non exists.
        if (!_impl->tasks.try_dequeue(task)) {
            _impl->idleNotficationCallback(_impl->id);
            _impl->tasks.wait_dequeue(task);
        }

        // A null task means that the core needs to shutdown.
        if (task == nullptr) {
            break;
        }

        task->Execute();

        // TODO @asib: perform post task cleanup operations.
    }
}

static v8::Isolate* CreateIsolate(const Settings& settings) {
    // The allocator is a global V8 setting.
    static napa::v8_extensions::ArrayBufferAllocator commonAllocator;

    v8::Isolate::CreateParams createParams;
    createParams.array_buffer_allocator = &commonAllocator;

    // Set the maximum V8 heap size.
    createParams.constraints.set_max_old_space_size(settings.maxOldSpaceSize);
    createParams.constraints.set_max_semi_space_size(settings.maxSemiSpaceSize);
    createParams.constraints.set_max_executable_size(settings.maxExecutableSize);

    return v8::Isolate::New(createParams);
}

static void ConfigureIsolate(v8::Isolate* isolate, const Settings& settings) {
    // TODO @asib: Configure GC and counters if needed.

    // Logs V8 error and prevent it from aborting.
    // The root cause of V8's fatal error must be caught and prevented.
    isolate->SetFatalErrorHandler([](const char* location, const char* message) {
        // TODO @asib: log error message
    });

    // Prevent V8 from aborting on uncaught exception.
    isolate->SetAbortOnUncaughtExceptionCallback([](v8::Isolate*) {
        return false;
    });

    // V8 takes a pointer to the minimum (x86 stack grows down) allowed stack address
    // so, capture the current top of the stack and calculate minimum allowed
    uint32_t currentStackAddress;
    auto limit = (reinterpret_cast<uintptr_t>(&currentStackAddress - settings.maxStackSize / sizeof(uint32_t*)));
    isolate->SetStackLimit(limit);
}
