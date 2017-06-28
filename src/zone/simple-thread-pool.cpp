#include "simple-thread-pool.h"

using namespace napa::zone;

SimpleThreadPool::Worker::Worker(SimpleThreadPool& pool) : _pool(pool) {}

void SimpleThreadPool::Worker::operator()() {
    std::function<void()> task;

    while (true) {
        {
            std::unique_lock<std::mutex> lock(_pool._queueLock);
            if (!_pool._isStopped) {
                _pool._queueCondition.wait(lock, [this]() {
                    return _pool._isStopped || !_pool._taskQueue.empty();
                });
            }

            // Drain all existing tasks before actualy stopping.
            if (_pool._isStopped && _pool._taskQueue.empty()) {
                break;
            }

            task = std::move(_pool._taskQueue.front());
            _pool._taskQueue.pop();
        }

        task();
    }
}

SimpleThreadPool::SimpleThreadPool(uint32_t numberOfWorkers) : _isStopped(false) {
    _workers.reserve(numberOfWorkers);

    for (uint32_t i = 0; i < numberOfWorkers; ++i) {
        _workers.emplace_back(Worker(*this));
    }
}

SimpleThreadPool::~SimpleThreadPool() {
    {
        std::unique_lock<std::mutex> lock(_queueLock);
        _isStopped = true;
    }

    _queueCondition.notify_all();

    for (auto& thread : _workers) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}
