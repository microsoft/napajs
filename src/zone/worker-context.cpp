#include "worker-context.h"

#include <napa-log.h>

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
}

void* WorkerContext::Get(WorkerContextItem item) {
    return GetTlsData(item);
}

void WorkerContext::Set(WorkerContextItem item, void* data) {
    GetTlsData(item) = data;
}