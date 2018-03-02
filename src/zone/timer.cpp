// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "timer.h"

#include <napa/log.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <stack>
#include <thread>

#include <iostream>

using namespace napa::zone;

struct TimerInfo {
    bool active;
    std::chrono::milliseconds timeout;
    Timer::Callback callback;
};

struct ActiveTimerEntry {
    Timer::Index index;
    std::chrono::high_resolution_clock::time_point expirationTime;
};

static bool operator<(const ActiveTimerEntry& first, const ActiveTimerEntry& second) {
    // We want the entry that is closest to expire to be at the top of the queue.
    return first.expirationTime > second.expirationTime;
}

struct TimersScheduler {
    ~TimersScheduler();

    bool StartMainLoop();

    std::priority_queue<ActiveTimerEntry> activeTimers;
    std::stack<Timer::Index> freeSlots;

    std::vector<TimerInfo> timers;

    std::condition_variable cv;
    std::mutex mutex;
    std::atomic<bool> running;

    std::thread thread;
};

TimersScheduler::~TimersScheduler() {
    running = false;
    cv.notify_one();

    if (thread.joinable()) {
        thread.join();
    }
}

bool TimersScheduler::StartMainLoop() {
    running = true;
    thread = std::thread([this]() {
        std::unique_lock<std::mutex> lock(mutex);

        // Timers main loop.
        while (running) {
            cv.wait(lock, [this]() {
                return !this->activeTimers.empty() || !this->running;
            });

            if (!running) {
                return;
            }

            auto nextExpirationTime = activeTimers.top().expirationTime;
            if (nextExpirationTime <= std::chrono::high_resolution_clock::now()) {
                // Pop before callback）to re-arm the timer.
                auto expiredTimer = activeTimers.top();
                activeTimers.pop();

                if (timers[expiredTimer.index].active) {
                    timers[expiredTimer.index].active = false;

                    try {
                        // Fire the callback. The callback is assumed to be 
                        // VERY VERY FAST as it is meant to dispatch to appropriate 
                        // callback queues. So do not free lock here.
                        timers[expiredTimer.index].callback();
                    }
                    catch (const std::exception &ex) {
                        LOG_ERROR("Timers", "Timer callback threw an exception. %s", ex.what());
                    }
                }
            }
            else {
                // Wait for timer expiration. Stop waiting if new urgent active timer is arm-ed.
                cv.wait_until(lock, nextExpirationTime, [this, nextExpirationTime]() {
                    return this->activeTimers.top().expirationTime < nextExpirationTime;
                });
            }
        }
    });

    return true;
}

static TimersScheduler _timersScheduler;


Timer::Timer(Callback callback, std::chrono::milliseconds timeout) {
    // Start the timers scheduler if this is the first timer created.
    static bool init = _timersScheduler.StartMainLoop();

    std::lock_guard<std::mutex> lock(_timersScheduler.mutex);

    TimerInfo timerInfo{ false, timeout, callback };

    if (!_timersScheduler.freeSlots.empty()) {
        _index = _timersScheduler.freeSlots.top();
        _timersScheduler.freeSlots.pop();

        _timersScheduler.timers[_index] = std::move(timerInfo);
    }
    else {
        _index = static_cast<Timer::Index>(_timersScheduler.timers.size());
        _timersScheduler.timers.emplace_back(timerInfo);
    }
}

Timer::~Timer() {
    std::lock_guard<std::mutex> lock(_timersScheduler.mutex);

    _timersScheduler.timers[_index].active = false;

    // Free the timer slot.
    _timersScheduler.freeSlots.emplace(_index);
}

void Timer::Start() {
    {
        std::lock_guard<std::mutex> lock(_timersScheduler.mutex);
        
        auto& timerInfo = _timersScheduler.timers[_index];
        timerInfo.active = true;

        ActiveTimerEntry entry = { _index, std::chrono::high_resolution_clock::now() + timerInfo.timeout };
        _timersScheduler.activeTimers.emplace(std::move(entry));
    }

    _timersScheduler.cv.notify_one();
}

void Timer::Stop() {
    std::lock_guard<std::mutex> lock(_timersScheduler.mutex);

    _timersScheduler.timers[_index].active = false;
}
