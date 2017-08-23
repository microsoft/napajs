// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <stdint.h>
#include <thread>
#include <utility>
#include <vector>

namespace napa {
namespace zone {

    /// <summary> Simple thread pool. </summary>
    class SimpleThreadPool {
    public:
        /// <summary> Constructor. </summary>
        /// <param name="numberOfWorkers"> Number of workers. </param>
        explicit SimpleThreadPool(uint32_t numberOfWorkers);

        /// <summary> Destructor. </summary>
        virtual ~SimpleThreadPool();

        /// <summary> Execute the given function in one of the workers. </summary>
        /// <param name="function"> Function to run. </param>
        /// <param name="args"> Arguments for function. </param>
        template <typename T, typename... Args>
        void Execute(T&& function, Args&&... args);

    private:
        /// <summary> Class for worker main loop. </summary>
        class Worker {
        public:
            /// <summary> Constructor. </summary>
            Worker(SimpleThreadPool& pool);

            /// <summary> Main thread function. </summary>
            void operator()();

        private:
            /// <summary> Thread pool instance. </summary>
            SimpleThreadPool& _pool;
        };

        std::vector<std::thread> _workers;
        std::queue<std::function<void()>> _taskQueue;

        /// <summary> Critical section and event for task queue. </summary>
        std::mutex _queueLock;
        std::condition_variable _queueCondition;

        /// <summary> Flag to stop threads. </summary>
        bool _isStopped;
    };


    template <typename T, typename... Args>
    void SimpleThreadPool::Execute(T&& function, Args&&... args) {
        auto task = std::bind(std::forward<T>(function), std::forward<Args>(args)...);

        {
            std::unique_lock<std::mutex> lock(_queueLock);
            _taskQueue.emplace(task);
        }

        _queueCondition.notify_one();
    }

} // namespace zone
} // namespace napa