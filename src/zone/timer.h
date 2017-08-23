// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <chrono>
#include <functional>
#include <memory>

namespace napa {
namespace zone {

    /// <summary> A timer class that will trigger a callback after elapsed time. </summary>
    class Timer {
    public:
        typedef uint16_t Index;
        typedef std::function<void(void)> Callback;

        /// <summary> Creates a new timer which is not active initially. </summary>
        /// <param name="callback"> The callback. </param>
        /// <param name="timeout"> The timeout in millisecond after which the callback will be triggered. </param>
        Timer(Callback callback, std::chrono::milliseconds timeout);

        /// <summary> Destructor. Stops the timer. </summary>
        ~Timer();

        /// <summary> Activates the timer to trigger the callback after specified milliseconds. </summary>
        void Start();

        /// <summary> Disables the timer, preventing the calback from triggering. </summary>
        void Stop();

    private:
        Index _index;
    };
} // namespace zone
} // namespace napa