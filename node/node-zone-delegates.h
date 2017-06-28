#pragma once

#include <napa/types.h>

namespace napa {
namespace node_zone {

    /// <summary> Broadcast to Node zone. </summary>
    void Broadcast(const std::string& source, napa::BroadcastCallback callback);

    /// <summary> Execute in Node zone. </summary>
    void Execute(const napa::FunctionSpec& spec, napa::ExecuteCallback callback);
}
}
