// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/exports.h>

#include "zone.h"

#include <functional>

namespace v8 {
    class TaskRunner;
}

namespace napa {
namespace zone {

    /// <summary> Delegate for Broadcast on Node zone. </summary>
    using BroadcastDelegate = std::function<void(const FunctionSpec&, BroadcastCallback, v8::TaskRunner*)>;

    /// <summary> Delegate for Execute on Node zone. </summary>
    using ExecuteDelegate = std::function<void(const FunctionSpec&, ExecuteCallback, v8::TaskRunner*)>;

    /// <summary> Concrete implementation of a Node zone. </summary>
    class NodeZone : public Zone {
    public:
        /// <summary> Set delegate function for Broadcast and Execute on node zone. This is intended to be called from napa-binding.node. </summary>
        static NAPA_API void Init(BroadcastDelegate broadcast, ExecuteDelegate execute);

        /// <summary> 
        ///    Retrieves an existing zone. 
        ///    If Node is not applicable (like in embed mode), a nullptr will be returned.
        /// </summary>
        static std::shared_ptr<NodeZone> Get();

        /// <see cref="Zone::GetId" />
        virtual const std::string& GetId() const override;

        /// <see cref="Zone::Broadcast" />
        virtual void Broadcast(const FunctionSpec& spec, BroadcastCallback callback) override;

        /// <see cref="Zone::Execute" />
        virtual void Execute(const FunctionSpec& spec, ExecuteCallback callback) override;

    private:
        /// <summary> Constructor. </summary>
        NodeZone(BroadcastDelegate broadcast, ExecuteDelegate execute);

        /// <summary> Broadcast delegate for node zone. </summary>
        BroadcastDelegate _broadcast;

        /// <summary> Execute delegate for node zone. </summary>
        ExecuteDelegate _execute;

        /// <summary> Node zone id. </summary>
        std::string _id;

        v8::TaskRunner* _foregroundTaskRunner;

        v8::TaskRunner* _backgroundTaskRunner;

        /// <summary> Node zone instance. </summary>
        static std::shared_ptr<NodeZone> _instance;
    };
}
}