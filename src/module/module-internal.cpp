#include <napa/module-internal.h>
#include <napa/thread-local-storage.h>

#include <napa-log.h>

using namespace napa;
using namespace napa::module;

// Global instance of IsolateData.
IsolateData& IsolateData::GetInstance() {
    static IsolateData isolateData;
    return isolateData;
}

IsolateData::IsolateData() {
    for (auto& tlsIndex : _tlsIndexes) {
        tlsIndex = tls::Alloc();
    }
}

IsolateData::~IsolateData() {
    for (auto tlsIndex : _tlsIndexes) {
        tls::Free(tlsIndex);
    }
}

void IsolateData::Init() {
    for (auto tlsIndex : GetInstance()._tlsIndexes) {
        tls::SetValue(tlsIndex, nullptr);
    }
}

void* IsolateData::Get(IsolateDataId isolateDataId) {
    auto slotId = static_cast<size_t>(isolateDataId);
    NAPA_ASSERT(slotId < GetInstance()._tlsIndexes.size(), "slot id out of range");

    auto tlsIndex = GetInstance()._tlsIndexes[slotId];
    return tls::GetValue(tlsIndex);
}

void IsolateData::Set(IsolateDataId isolateDataId,
                      void* data) {
    auto slotId = static_cast<size_t>(isolateDataId);
    NAPA_ASSERT(slotId < GetInstance()._tlsIndexes.size(), "slot id out of range");

    auto tlsIndex = GetInstance()._tlsIndexes[slotId];
    tls::SetValue(tlsIndex, data);
}