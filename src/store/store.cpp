#include "store.h"

#include <napa-memory.h>
#include <napa/stl/unordered_map.h>

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>

using namespace napa::store;

class StoreImpl: public Store {
public:
    /// <summary> Consturctor. </summary>
    explicit StoreImpl(const char* id)
        : _id(id) {
    }

    /// <summary> Get ID of this store. </summary>
    const char* GetId() const override {
        return _id.c_str();
    }

    /// <summary> Set value with a key. </summary>
    /// <param name="key"> Case-sensitive key to set. </param>
    /// <param name="value"> Pair of payload and transport context. </returns>
    void Set(const char* key, Store::ValueType value) override {
        auto it = _valueMap.find(key);
        if (it != _valueMap.end()) {
            it->second = std::move(value);
        } else {
            _valueMap.emplace(napa::stl::String(key), std::move(value));
        }
    }

    /// <summary> Get value by a key. </summary>
    /// <param name="key"> Case-sensitive key to get. </param>
    /// <returns> ValueType pointer, null if not found. </returns>
    const ValueType* Get(const char* key) const override {
        auto it = _valueMap.find(key);
        if (it != _valueMap.end()) {
            return &(it->second);
        }
        return nullptr;
    }

    /// <summary> Check if this store has a key. </summary>
    /// <param name="key"> Case-sensitive key. </param>
    /// <returns> True if the key exists in store. </returns>
    bool Has(const char* key) const override {
        return _valueMap.find(key) != _valueMap.end();
    }

    /// <summary> Delete a key. No-op if key is not found in store. </summary>
    void Delete(const char* key) override {
        _valueMap.erase(key);
    }

    /// <summary> Return size of the store. </summary>
    size_t Size() const override {
        return _valueMap.size();
    }

private:
    /// <summary> ID. Case sensitive. </summary>
    napa::stl::String _id;

    /// <summary> Key to value map. </summary>
    napa::stl::UnorderedMap<napa::stl::String, Store::ValueType> _valueMap;
};

namespace napa {
namespace store {

    namespace {
        napa::stl::UnorderedMap<napa::stl::String, std::weak_ptr<Store>> _storeRegistry;
        boost::shared_mutex _registryAccess;
    } // namespace

    std::shared_ptr<Store> CreateStore(const char* id) {
        boost::unique_lock<boost::shared_mutex> lockWrite(_registryAccess);
        
        std::shared_ptr<Store> store;
        auto it = _storeRegistry.find(id);
        if (it == _storeRegistry.end()) {
            store = NAPA_MAKE_SHARED<StoreImpl>(id);
            _storeRegistry.insert(std::make_pair(napa::stl::String(id), store));
        }
        return store;
    }

    std::shared_ptr<Store> GetOrCreateStore(const char* id) {
        auto store = GetStore(id);
        if (store == nullptr) {
            store = CreateStore(id);
            if (store == nullptr) {
                // Already created just now. Lookup again.
                store = GetStore(id);
            }
        }
        return store;
    }

    std::shared_ptr<Store> GetStore(const char* id) {
        boost::shared_lock<boost::shared_mutex> lockRead(_registryAccess);
        auto it = _storeRegistry.find(id);
        if (it != _storeRegistry.end()) {
            return it->second.lock();
        }
        return std::shared_ptr<Store>();
    }

    size_t GetStoreCount() {
        boost::unique_lock<boost::shared_mutex> lockWrite(_registryAccess);
        for (auto it = _storeRegistry.begin(); it != _storeRegistry.end(); ) {
            if (it->second.use_count() == 0) {
                _storeRegistry.erase(it++);
            } else {
                ++it;
            }
        }
        return _storeRegistry.size();
    }
} // namespace store
} // namespace napa
