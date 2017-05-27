#pragma once

#include <napa/exports.h>
#include <napa/stl/string.h>
#include <napa/transport/transport-context.h>
#include <memory>

namespace napa {
namespace memory {

    /// <summary> Class for memory store, which stores transportable JS objects across isolates. </summary>
    /// <remarks> Store is not thread-safe, and is intended to be used by StoreWrap. 
    /// We expose Store in napa.dll instead of napa-binding for sharing memory between Napa and Node.JS. </remarks>
    class Store {
    public:
        /// Meta-data that is necessary to marshall/unmarshall JS values.
        struct ValueType {
            /// <summary> JSON string from marshalled JS value. </summary>
            napa::stl::String payload;

            /// <summary> TransportContext that is needed to unmarshall the JS value. </summary>
            napa::transport::TransportContext transportContext;
        };

        /// <summary> Get ID of this store. </summary>
        virtual const char* GetId() const = 0;

        /// <summary> Set value with a key. </summary>
        /// <param name="key"> Case-sensitive key to set. </param>
        /// <param name="value"> Pair of payload and transport context. </returns>
        virtual void Set(const char* key, ValueType value) = 0;

        /// <summary> Get value by a key. </summary>
        /// <param name="key"> Case-sensitive key to get. </param>
        /// <returns> ValueType pointer, null if not found. </returns>
        virtual const ValueType* Get(const char* key) const = 0;

        /// <summary> Check if this store has a key. </summary>
        /// <param name="key"> Case-sensitive key. </param>
        /// <returns> True if the key exists in store. </returns>
        virtual bool Has(const char* key) const = 0;

        /// <summary> Delete a key. No-op if key is not found in store. </summary>
        virtual void Delete(const char* key) = 0;

        /// <summary> Return size of the store. </summary>
        virtual size_t Size() const = 0;

        /// <summary> Destructor. </summary>
        virtual ~Store() = default;
    };

    /// <summary> Create a store by id. </summary>
    /// <param name="id"> Case-sensitive id. </summary>
    /// <returns> Newly created store, or nullptr if store associated with id already exists. </summary>
    NAPA_API std::shared_ptr<Store> CreateStore(const char* id);

    /// <summary> Get or create a store by id. </summary>
    /// <param name="id"> Case-sensitive id. </summary>
    /// <returns> Existing or newly created store. Should never be nullptr. </summary>
    NAPA_API std::shared_ptr<Store> GetOrCreateStore(const char* id);

    /// <summary> Get a store by id. </summary>
    /// <param name="id"> Case-sensitive id. </summary>
    /// <returns> Existing store or nullptr if not found. </summary>
    NAPA_API std::shared_ptr<Store> GetStore(const char* id);

    /// <summary> Get store count currently in use. </summary>
    NAPA_API size_t GetStoreCount();
}
}