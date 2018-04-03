// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/types.h>

namespace napa {
namespace node_zone {

    /// <summary> Broadcast to Node zone. </summary>
    void Broadcast(const napa::FunctionSpec& spec, napa::BroadcastCallback callback);

    /// <summary> Execute in Node zone. </summary>
    void Execute(const napa::FunctionSpec& spec, napa::ExecuteCallback callback);
}
}
