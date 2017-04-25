#pragma once

#include <napa/module/napa-async-runner.h>
#include <scheduler/worker.h>
#include <zone/zone-impl.h>

#include <v8.h>

#include <future>

namespace napa {
namespace module {
    
    /// <summary> Class holding asynchonous callbacks. </summary>
    struct AsyncContext {
        /// <summary> Zone instance issueing asynchronous work. </summary>
        ZoneImpl* zone;

        /// <summary> Worker Id issueing asynchronous work. </summary>
        scheduler::WorkerId workerId;

        /// <summary> Future to wait async callback. </summary>
        std::future<void> future;

        /// <summary> Javascript callback. </summary>
        v8::Persistent<v8::Function> jsCallback;

        /// <summary> Function to run asynchronously in separate thread. </summary>
        AsyncWork asyncWork;

        /// <summary> Callback running in V8 isolate after asynchronous callback completes. </summary>
        AsyncCompleteCallback asyncCompleteCallback;
    };

}   // End of namespace scheduler.
}   // End of namespace napa.