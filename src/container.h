#ifndef NAPA_CONTAINER_H
#define NAPA_CONTAINER_H

#include "settings.h"
#include "scheduler.h"

#include <memory>

namespace napa {
namespace runtime {
namespace internal {

    class Container {
    public:
        void Initialize(Settings&& settings);

        // TODO @asib: add all container methods

    private:
        Settings _settings;
        std::unique_ptr<Scheduler> _scheduler;
    };
}
}
}

#endif //NAPA_CONTAINER_H
