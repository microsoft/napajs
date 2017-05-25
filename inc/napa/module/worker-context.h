#pragma once

#include <napa/exports.h>

#include <array>

namespace napa {
namespace module {

    /// <summary> Worker context item to store Napa specific data for a module to be able to access. </summary>
    enum class WorkerContextItem : uint32_t {
        /// <summary> Isolate instance. </summary>
        ISOLATE = 0,

        /// <summary> Module's persistent constructor object. </summary>
        CONSTRUCTOR,

        /// <summary> Module loader instance. </summary>
        MODULE_LOADER,

        /// <summary> Module object for Napa binding. It will be filled under both Napa and Node isolate. </summary>
        NAPA_BINDING,

        /// <summary> Zone instance. </summary>
        ZONE,

        /// <summary> Worker Id. </summary>
        WORKER_ID,

        /// <summary> End of index. </summary>
        END_OF_WORKER_CONTEXT_ITEM
    };

    /// <summary> Napa specific data stored at TLS. </summary>
    class NAPA_API WorkerContext {
    public:

        /// <summary> Returns the single instance of this class. </summary>
        static WorkerContext& GetInstance();

        /// <summary> Initialize isolate data. </summary>
        static void Init();

        /// <summary> Get stored TLS data. </summary>
        /// <param name="item"> Pre-defined data id for Napa specific data. </param>
        /// <returns> Stored TLS data. </returns>
        static void* Get(WorkerContextItem item);

        /// <summary> Set TLS data into the given slot. </summary>
        /// <param name="item"> Pre-defined data id for Napa specific data. </param>
        /// <param name="data"> Pointer to stored data. </param>
        static void Set(WorkerContextItem item, void* data);

    private:

        /// <summary> Constructor to assign TLS index to all data. It's done once at process level. </summary>
        WorkerContext();

        /// <summary> Destructor. </summary>
        ~WorkerContext();

        /// <summary> It stores the tls index for all data. </summary>
        std::array<uint32_t, static_cast<size_t>(WorkerContextItem::END_OF_WORKER_CONTEXT_ITEM)> _tlsIndexes;
    };

    #define INIT_WORKER_CONTEXT napa::module::WorkerContext::Init

}   // End of namespace module.
}   // End of namespace napa.