#include <napa/module/worker-context.h>
#include <napa/module/thread-local-storage.h>

#include <napa-log.h>

using namespace napa;
using namespace napa::module;

// Global instance of WorkerContext.
WorkerContext& WorkerContext::GetInstance() {
    static WorkerContext context;
    return context;
}

WorkerContext::WorkerContext() {
    for (auto& tlsIndex : _tlsIndexes) {
        tlsIndex = tls::Alloc();
    }
}

WorkerContext::~WorkerContext() {
    for (auto tlsIndex : _tlsIndexes) {
        tls::Free(tlsIndex);
    }
}

void WorkerContext::Init() {
    for (auto tlsIndex : GetInstance()._tlsIndexes) {
        tls::SetValue(tlsIndex, nullptr);
    }
}

void* WorkerContext::Get(WorkerContextItem item) {
    auto slotId = static_cast<size_t>(item);
    NAPA_ASSERT(slotId < GetInstance()._tlsIndexes.size(), "slot id out of range");

    auto tlsIndex = GetInstance()._tlsIndexes[slotId];
    return tls::GetValue(tlsIndex);
}

void WorkerContext::Set(WorkerContextItem item, void* data) {
    auto slotId = static_cast<size_t>(item);
    NAPA_ASSERT(slotId < GetInstance()._tlsIndexes.size(), "slot id out of range");

    auto tlsIndex = GetInstance()._tlsIndexes[slotId];
    tls::SetValue(tlsIndex, data);
}