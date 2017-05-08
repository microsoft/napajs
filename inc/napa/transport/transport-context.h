#pragma once

#include <napa/stl/unordered_map.h>
#include <memory>

namespace napa {
namespace transport {
    
    /// <summary> It facilitates transportation of C++ objects 
    /// which need to transfer/extend their ownership across isolates by assignment operator. 
    ///
    /// Known cases are:
    /// 1) Transfer/extend ownership via passing shared objects in arguments to Zone::execute.
    /// 2) Extend ownership by cross-isolate sharing via memory.global.set/get or Zone.global.set/get.
    ///
    /// At this time, only transport std::shared_ptr is supported of transfering ownership. 
    /// </summary>
    class TransportContext {

    public:
        /// <summary> Default constructor. </summary>
        TransportContext() = default;

        /// <summary> Move constructor. </summary>
        TransportContext(TransportContext&& other) 
            : _sharedDepot(std::move(other._sharedDepot)) {
        }

        /// <summary> Move assignment. </summary>
        TransportContext& operator=(TransportContext&& other) {
            _sharedDepot = std::move(other._sharedDepot);
        }

        /// <summary> It saves a shared pointer that can be loaded later. </summary>
        /// <param name="pointer"> Shared pointer to transfer owership to another isolate. </param>
        template <typename T>
        void SaveShared(std::shared_ptr<T> pointer) {
            _sharedDepot[reinterpret_cast<uintptr_t>(pointer.get())] = std::move(pointer);
        }

        /// <summary> It loads a previously saved shared pointer. </summary>
        /// <param name="handle"> uintptr_t value. </summary>
        /// <returns> shared_ptr for requested handle, or empty shared_ptr if not found. </returns>
        template <typename T>
        std::shared_ptr<T> LoadShared(uintptr_t handle) {
            auto it = _sharedDepot.find(handle);
            if (it != _sharedDepot.end()) {
                return std::static_pointer_cast<T>(it->second);
            }
            return std::shared_ptr<T>();
        }

        /// <summary> Get count of saved shared_ptr. </summary> 
        uint32_t GetSharedCount() {
            return static_cast<uint32_t>(_sharedDepot.size());
        }

    private:
        /// <summary> Non-copyable. </summary>
        TransportContext(TransportContext&) = delete;
        TransportContext& operator=(TransportContext&) = delete;

        /// <summary> shared_ptr depot. </summary>
        napa::stl::UnorderedMap<uintptr_t, std::shared_ptr<void>> _sharedDepot;
    };
}
}
    