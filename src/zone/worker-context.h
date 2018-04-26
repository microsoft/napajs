// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/exports.h>

#include <array>

namespace napa {
namespace zone {

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

        /// <summary> Zone Id. </summary>
        ZONE_ID,

        /// <summary> Worker Id. </summary>
        WORKER_ID,

        /// <summary> Event loop. </summary>
        EVENT_LOOP,

        /// <summary> End of index. </summary>
        END_OF_WORKER_CONTEXT_ITEM
    };

    /// <summary> Napa specific data stored at TLS. </summary>
    class NAPA_API WorkerContext {
    public:

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
    };

    #define INIT_WORKER_CONTEXT napa::zone::WorkerContext::Init

}   // End of namespace zone.
}   // End of namespace napa.