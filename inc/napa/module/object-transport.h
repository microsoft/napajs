#pragma once

#include <array>
#include <memory>
#include <typeindex>

namespace napa {
namespace module {

    /// <summary>
    /// It makes a module to be able to use a host's native object.
    /// It doesn't guarantee that object destruction is perfectly detected by a module.
    /// So, Host application should make ObjectTransporter instance returned by Serialized() alive until a request is processed.
    /// </summary>
    class ObjectTransporter final {
    public:

        /// <summary> Constructor. </summary>
        /// <param name="object"> Shared pointer to object instance. </param>
        /// <param name="type"> Object type information. </param>
        /// <remarks> Serialize() plays a role of factory to create an instance. </remarks>
        ObjectTransporter(std::shared_ptr<void> object,
                          std::type_index type)
            : _object(object)
            , _type(type) {}

        /// <summary> Create an ObjectTransporter instance and its pointer value to pass to a module. </summary>
        /// <param name="object"> Object to be accessed by a module. </param>
        /// <returns> Pair of ObjectTransporter instance and pointer value. </returns>
        /// <remarks> Since a module doesn't know object's lifetime, this API is good to pass a global object. </remarks>
        template <typename T>
        static std::pair<uintptr_t, std::unique_ptr<ObjectTransporter>> Serialize(T* object) {
            return Serialize(std::shared_ptr<T>(object, [](T*) {}));
        }

        /// <summary> Create an ObjectTransporter instance and its pointer value to pass to a module. </summary>
        /// <param name="object"> Shared pointer type of object to be accessed by a module. </param>
        /// <returns> Pair of ObjectTransporter instance and pointer value. </returns>
        template <typename T>
        static std::pair<uintptr_t, std::unique_ptr<ObjectTransporter>> Serialize(std::shared_ptr<T> object) {
            auto transporter = std::make_unique<ObjectTransporter>(
                std::static_pointer_cast<void>(std::move(object)),
                typeid(T));
            return std::pair<uintptr_t, std::unique_ptr<ObjectTransporter>>(
                reinterpret_cast<uintptr_t>(transporter.get()),
                std::move(transporter));
        }

        /// <summary> Get a passing object instance from pointer value of ObjectTransporter. </summary>
        /// <param name="pointer"> Pointer value of ObjectTransporter instance. </param>
        /// <returns> Passing object instance. </returns>
        template <typename T>
        static std::shared_ptr<T> Deserialize(uintptr_t pointer) {
            auto transporter = reinterpret_cast<ObjectTransporter*>(pointer);
            if (std::type_index(typeid(T)) != transporter->_type) {
                return nullptr;
            }

            return std::static_pointer_cast<T>(transporter->_object);
        }

    private:

        /// <summary> Pointer to passing object. </summary>
        std::shared_ptr<void> _object;

        /// <summary> Type information of passing object. </summary>
        std::type_index _type;
    };

    template <typename T>
    class TransportableObject : public std::enable_shared_from_this<TransportableObject<T>> {
    public:

        std::pair<uintptr_t, std::unique_ptr<ObjectTransporter>> Serialize() {
            auto transporter = std::make_unique<ObjectTransporter>(
                std::static_pointer_cast<void>(shared_from_this()),
                typeid(T));
            return std::pair<uintptr_t, std::unique_ptr<ObjectTransporter>>(
                reinterpret_cast<uintptr_t>(transporter.get()),
                std::move(transporter));
        }
    };

    static const uint32_t UINTPTR_SIZE_IN_UINT32 = static_cast<uint32_t>(sizeof(uintptr_t) / sizeof(uint32_t)); 
}

namespace module {
namespace object_transport {

    /// <summary> Convert a pointer value to uint32 array. </summary>
    /// <param name="source"> Pointer value. </summary>
    /// <returns> uint32 array. </returns>
    inline std::array<uint32_t, UINTPTR_SIZE_IN_UINT32> UintptrToUint32Array(uintptr_t source) {
        std::array<uint32_t, UINTPTR_SIZE_IN_UINT32> target = {};
        for (uint32_t i = 0; i < UINTPTR_SIZE_IN_UINT32; ++i, source >>= 32) {
            target[i] = static_cast<uint32_t>(source);
        }
        return target;
    }

    /// <summary> Convert a uint32 array to uintptr. </summary>
    /// <param name="source"> a uint32 array holding pointer value. </summary>
    /// <returns> Converted pointer value. </returns>
    inline uintptr_t Uint32ArrayToUintptr(const std::array<uint32_t, UINTPTR_SIZE_IN_UINT32>& source) {
        uintptr_t result = 0;
        for (uint32_t i = 0; i < UINTPTR_SIZE_IN_UINT32; ++i) {
            result |= static_cast<uintptr_t>(source[i]) << 32 * i;
        }
        return result;
    }

}
}
}
