#include "shared-barrel.h"

using namespace napa::module;

SharedBarrel::SharedBarrel(IdType id) : _id(id) {}

void SharedBarrel::Invalidate() {
    boost::unique_lock<boost::shared_mutex> lock(_mutex);

    _id = INVALID_ID;
    _sharedObjects.clear();
}

SharedBarrel::IdType SharedBarrel::GetId() const {
    return _id;
}

bool SharedBarrel::Exists(const char* key) const {
    boost::shared_lock<boost::shared_mutex> lock(_mutex);

    return _sharedObjects.find(key) != _sharedObjects.end();
}

void SharedBarrel::Remove(const char* key) {
    boost::unique_lock<boost::shared_mutex> lock(_mutex);

    _sharedObjects.erase(key);
}

size_t SharedBarrel::GetCount() const {
    boost::shared_lock<boost::shared_mutex> lock(_mutex);

    return _sharedObjects.size();
}