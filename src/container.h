#pragma once

#include "settings/settings.h"
#include "scheduler/scheduler.h"

#include <memory>

namespace napa {
namespace runtime {
namespace internal {

    class Container {
    public:
        void Initialize(std::unique_ptr<Settings> settings);

        // TODO @asib: add all container methods

    private:
        std::unique_ptr<Settings> _settings;
        std::unique_ptr<Scheduler> _scheduler;
    };
}
}
}