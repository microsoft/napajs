#include "shared-depot.h"

#include <napa-log.h>

using namespace napa::module;

/// <remarks>
/// This API needs to return a shared pointer to SharedBarrel instance and also assign a unique Id.
/// To make both operations efficient, it uses two data structures, the vector of SharedBarrel instances
/// and the list of free SharedBarrel Ids. This way, we can assign the small range of numeric Ids
/// to a shared barrel and avoid using a map.
/// </remarks>
std::shared_ptr<SharedBarrel> SharedDepot::Assign() {
    boost::unique_lock<boost::shared_mutex> lock(_mutex);

    if (_freeList.empty()) {
        SharedBarrel::IdType index = static_cast<SharedBarrel::IdType>(_sharedBarrels.size());
        _sharedBarrels.resize(_sharedBarrels.size() + 64);
        auto capacity = _sharedBarrels.size();
        while (index < capacity) {
            _freeList.push(index++);
        }
    }

    auto id = _freeList.front();
    _freeList.pop();

    auto barrel = std::make_shared<SharedBarrel>(id);
    _sharedBarrels[id] = barrel;
    return barrel;
}

void SharedDepot::Release(const std::shared_ptr<SharedBarrel>& barrel) {
    NAPA_ASSERT(barrel != nullptr, "It tries to release an invalid shared barrel.");

    Release(barrel->GetId());
}

void SharedDepot::Release(SharedBarrel::IdType id) {
    NAPA_ASSERT(id < _sharedBarrels.size(), "Shared barrel Id is out of range.");

    boost::unique_lock<boost::shared_mutex> lock(_mutex);

    _sharedBarrels[id]->Invalidate();
    _sharedBarrels[id].reset();
    _freeList.push(id);
}

std::shared_ptr<SharedBarrel> SharedDepot::Find(SharedBarrel::IdType id) const {
    if (id >= _sharedBarrels.size()) {
        return nullptr;
    }

    boost::shared_lock<boost::shared_mutex> lock(_mutex);

    return _sharedBarrels[id];
}

size_t SharedDepot::GetCount() const {
    boost::shared_lock<boost::shared_mutex> lock(_mutex);

    return _sharedBarrels.size() - _freeList.size();
}