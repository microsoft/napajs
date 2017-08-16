// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "node-zone.h"

#include "worker-context.h"

#include <napa/assert.h>

using namespace napa;
using namespace napa::zone;

std::shared_ptr<NodeZone> NodeZone::_instance;

void NodeZone::Init(BroadcastDelegate broadcast, ExecuteDelegate execute) {
    _instance.reset(new NodeZone(broadcast, execute));
}

NodeZone::NodeZone(BroadcastDelegate broadcast, ExecuteDelegate execute)
    : _broadcast(std::move(broadcast)), _execute(std::move(execute)), _id("node") {

    NAPA_ASSERT(_broadcast, "Broadcast delegate must be a valid function.");
    NAPA_ASSERT(_execute, "Execute delegate must be a valid function.");

    // Init worker context for Node event loop.
    INIT_WORKER_CONTEXT();

    // Zone instance into TLS.
    WorkerContext::Set(WorkerContextItem::ZONE, reinterpret_cast<void*>(this));

    // Worker Id into TLS.
    WorkerContext::Set(WorkerContextItem::WORKER_ID, reinterpret_cast<void*>(static_cast<uintptr_t>(0)));
}

std::shared_ptr<NodeZone> NodeZone::Get() {
    return _instance;
}

const std::string& NodeZone::GetId() const {
    return _id;
}

void NodeZone::Broadcast(const std::string& source, BroadcastCallback callback) {
    _broadcast(source, callback);
}

void NodeZone::Execute(const FunctionSpec& spec, ExecuteCallback callback) {
    _execute(spec, callback);
}
