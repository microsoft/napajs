// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/types.h>

namespace napa {
namespace zone {

    /// <summary> Interface for Zone. </summary>
    struct Zone {

        /// <summary> Get the zone id. </summary>
        virtual const std::string& GetId() const = 0;

        /// <summary> Executes a pre-loaded JS function on all zone workers asynchronously. </summary>
        /// <param name="spec"> The function spec. </param>
        /// <param name="callback"> A callback that is triggered when broadcasting is done. </param>
        virtual void Broadcast(const FunctionSpec& spec, BroadcastCallback callback) = 0;

        /// <summary> Executes a pre-loaded JS function asynchronously. </summary>
        /// <param name="spec"> The function spec. </param>
        /// <param name="callback"> A callback that is triggered when execution is done. </param>
        virtual void Execute(const FunctionSpec& spec, ExecuteCallback callback) = 0;

        /// <summary> Virtual destructor. </summary>
        virtual ~Zone() {}
    };
}
}