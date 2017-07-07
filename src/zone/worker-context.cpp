#include "worker-context.h"

#include <napa-log.h>
#include <utils/debug.h>
#include <platform/platform.h>
#include <array>

using namespace napa::zone;

namespace {
    thread_local std::array<
        void*,
        static_cast<size_t>(WorkerContextItem::END_OF_WORKER_CONTEXT_ITEM)> dataCollection;

    // Get data by worker context item from the singleton TLS data collection
    void*& GetTlsData(WorkerContextItem item) {
        auto slotId = static_cast<size_t>(item);
        NAPA_ASSERT(slotId < dataCollection.size(), "slot id out of range");

        return dataCollection[slotId];
    }
}

void WorkerContext::Init() {
    dataCollection.fill(nullptr);
    NAPA_DEBUG("TLS", "[tid:%d] Init", platform::Gettid());
}

void* WorkerContext::Get(WorkerContextItem item) {
    auto data = GetTlsData(item);
    NAPA_DEBUG("TLS", "[tid:%d] Get(%d): %ld", platform::Gettid(), (int)item, (size_t)data);

    return data;
}

void WorkerContext::Set(WorkerContextItem item, void* data) {
    GetTlsData(item) = data;
    NAPA_DEBUG("TLS", "[tid:%d] Set(%d, %ld)", platform::Gettid(), (int)item, (size_t)data);
}