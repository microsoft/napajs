#include "timer.h"

#include <napa-log.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <stack>
#include <thread>

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

    thread.join();
}

bool TimersScheduler::StartMainLoop() {
    running = true;
    thread = std::thread([this]() {

        // Timers main loop.
        while (running) {
            std::unique_lock<std::mutex> lock(mutex);

            cv.wait(lock, [this]() {
                return !activeTimers.empty() || !running;
            });

            if (!running) {
                return;
            }

            auto now = std::chrono::high_resolution_clock::now();

            auto topTimer = activeTimers.top();
            if (topTimer.expirationTime < now) {
                // Timer expired, call callback if still valid.
                activeTimers.pop();

                if (timers[topTimer.index].active) {
                    try {
                        // Fire the callback.
                        // The callback is asusmed to be very fast as it is meant to dispatch to appropriate
                        // callback queues.
                        timers[topTimer.index].callback();
                    }
                    catch (const std::exception &ex) {
                        LOG_ERROR("Timers", "Timer callback threw an exception. %s", ex.what());
                    }

                    // We only support single trigger timers.
                    timers[topTimer.index].active = false;
                }
            }
            else {
                // top timer time have not elpased yet, wait for it.
                cv.wait_for(lock, topTimer.expirationTime - now);
            }
        }
    });

    return true;
}

static TimersScheduler _timersScheduler;


Timer::Timer(Callback callback, std::chrono::milliseconds timeout) {
    // Start the timers scheduler if this is the first timer created.
    static bool init = _timersScheduler.StartMainLoop();

    std::unique_lock<std::mutex> lock(_timersScheduler.mutex);

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
    Stop();

    std::unique_lock<std::mutex> lock(_timersScheduler.mutex);

    // Free the timer slot.
    _timersScheduler.freeSlots.emplace(_index);
}

void Timer::Start() {
    std::unique_lock<std::mutex> lock(_timersScheduler.mutex);

    auto& timerInfo = _timersScheduler.timers[_index];
    timerInfo.active = true;

    ActiveTimerEntry entry = { _index, std::chrono::high_resolution_clock::now() + timerInfo.timeout };
    _timersScheduler.activeTimers.emplace(std::move(entry));

    _timersScheduler.cv.notify_one();
}

void Timer::Stop() {
    std::lock_guard<std::mutex> lock(_timersScheduler.mutex);

    _timersScheduler.timers[_index].active = false;
}
