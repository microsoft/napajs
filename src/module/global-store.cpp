#include "global-store.h"

#include <napa-log.h>

#include <mutex>
#include <string>
#include <unordered_map>

using GlobalStore = std::unordered_map<std::string, void*>;

static std::mutex _mutex;

static GlobalStore& GetGlobalStore() {
    static GlobalStore store;

    return store;
}

void* napa::module::global_store::GetValue(const char* key) {
    std::lock_guard<std::mutex> lock(_mutex);

    auto& store = GetGlobalStore();

    auto iter = store.find(key);
    if (iter != store.end()) {
        return iter->second;
    }

    return nullptr;
}

bool napa::module::global_store::SetValue(const char* key, void* value) {
    std::lock_guard<std::mutex> lock(_mutex);

    auto& store = GetGlobalStore();

    auto res = store.emplace(key, value);
    if (!res.second) {
        LOG_ERROR("GlobalStore", "A value with key: %s already exists", key);
    }

    return res.second;
}
