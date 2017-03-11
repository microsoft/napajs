#pragma once

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <limits>
#include <string>
#include <unordered_map>

namespace napa {
namespace module {

    /// <summary>
    /// This class holds multiple SharedWrap instances.
    /// Each shared barrel has a unique id assigned by shared depot.
    /// All operations run in thread safe manner.
    /// </summary>
    class SharedBarrel {
    public:

        using IdType = uint32_t;
        static const IdType INVALID_ID = std::numeric_limits<IdType>::max();

        /// <summary> Constructor with Id. </summary>
        explicit SharedBarrel(IdType id);

        /// <summary> Invalidate this barrel. </summary>
        void Invalidate();

        /// <summary> It returns an Id. </summary>
        /// <returns> Barral Id. </returns>
        IdType GetId() const;

        /// <summary> It checks whether a shared object with a given key exists or not. </summary>
        /// <param name="key"> Shared object name given by owner. </param>
        /// <returns> True if the same key exists, otherwise false. </returns>
        bool Exists(const char* key) const;

        /// <summary> It stores a shared object into a barrel. </summary>
        /// <param name="key"> Shared object name. </param>
        /// <param name="object"> Shared object name given by owner. </param>
        /// <returns> True if it successfully adds an object into a barrel, otherwise false. </returns>
        /// <remarks> It returns false when the same key exists at a barrel. </remarks>
        template <typename T>
        bool Set(const char* key, std::shared_ptr<T> object);

        /// <summary> It returns a shared object from a barrel. </summary>
        /// <param name="key"> Shared object name given by owner. </param>
        /// <returns> Shared object. </returns>
        template <typename T>
        std::shared_ptr<T> Get(const char* key) const;

        /// <summary> It removes a shared object from a barrel. </summary>
        /// <param name="key"> Shared object name given by owner. </param>
        void Remove(const char* key);

        /// <summary> It returns the number of holding shared objects. </summary>
        size_t GetCount() const;

    private:

        using SharedObjectMap = std::unordered_map<std::string, std::shared_ptr<void>>;

        /// <summary> Shared barrel Id. </summary>
        IdType _id;

        /// <summary> Shared object map. </summary>
        SharedObjectMap _sharedObjects;

        /// <summary> Critical section for shared object map. </summary>
        mutable boost::shared_mutex _mutex;
    };

    template <typename T>
    bool SharedBarrel::Set(const char* key, std::shared_ptr<T> object) {
        boost::unique_lock<boost::shared_mutex> lock(_mutex);

        auto result = _sharedObjects.emplace(key, std::static_pointer_cast<void>(std::move(object)));
        return result.second;
    }

    template <typename T>
    std::shared_ptr<T> SharedBarrel::Get(const char* key) const {
        boost::shared_lock<boost::shared_mutex> lock(_mutex);

        auto iter = _sharedObjects.find(key);
        return (iter != _sharedObjects.end()) ? std::static_pointer_cast<T>(iter->second) : nullptr;
    }

}
}