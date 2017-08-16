// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/zone/napa-async-runner.h>

#include <zone/worker.h>
#include <zone/napa-zone.h>

#include <v8.h>

#include <future>

namespace napa {
namespace zone {

    /// <summary> Class holding asynchronous callbacks. </summary>
    struct AsyncContext {
        /// <summary> Zone instance issueing asynchronous work. </summary>
        NapaZone* zone = nullptr;

        /// <summary> Keep scheduler instance referenced until async work completes. </summary>
        std::shared_ptr<zone::Scheduler> scheduler;

        /// <summary> Worker Id issueing asynchronous work. </summary>
        zone::WorkerId workerId;

        /// <summary> Future to wait async callback. </summary>
        std::future<void> future;

        /// <summary> Javascript callback. </summary>
        v8::Persistent<v8::Function> jsCallback;

        /// <summary> Function to run asynchronously in separate thread. </summary>
        AsyncWork asyncWork;

        /// <summary> Return value from asynchronous work. </summary>
        void* result = nullptr;

        /// <summary> Callback running in V8 isolate after asynchronous callback completes. </summary>
        AsyncCompleteCallback asyncCompleteCallback;
    };

} // End of namespace zone.
} // End of namespace napa.
