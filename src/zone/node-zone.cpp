// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "node-zone.h"

#include "worker-context.h"

#include <napa/assert.h>

#include <node.h>
#include <uv.h>

using namespace napa;
using namespace napa::zone;

std::shared_ptr<NodeZone> NodeZone::_instance;

void NodeZone::Init(BroadcastDelegate broadcast, ExecuteDelegate execute) {
    // No lock is needed here, because the 1st time Init is called can be triggered
    // only by node main isolate, which should not be interupted.
    if (_instance == nullptr) {
        _instance.reset(new NodeZone(broadcast, execute));
    }
}

NodeZone::NodeZone(BroadcastDelegate broadcast, ExecuteDelegate execute):
    _broadcast(std::move(broadcast)), _execute(std::move(execute)), _id("node") {

    NAPA_ASSERT(_broadcast, "Broadcast delegate must be a valid function.");
    NAPA_ASSERT(_execute, "Execute delegate must be a valid function.");

    _foregroundTaskRunner = node::GetNodeIsolateForegroundTaskRunner();
    _backgroundTaskRunner = node::GetNodeIsolateBackgroundTaskRunner();

    // Init worker context for Node event loop.
    INIT_WORKER_CONTEXT();

    // Zone instance into TLS.
    WorkerContext::Set(WorkerContextItem::ZONE, reinterpret_cast<void*>(this));

    // Worker Id into TLS.
    WorkerContext::Set(WorkerContextItem::WORKER_ID, reinterpret_cast<void*>(static_cast<uintptr_t>(0)));

    // Set event loop into TLS. </summary>
    WorkerContext::Set(WorkerContextItem::EVENT_LOOP, reinterpret_cast<void*>(uv_default_loop()));
}

std::shared_ptr<NodeZone> NodeZone::Get() {
    return _instance;
}

const std::string& NodeZone::GetId() const {
    return _id;
}

void NodeZone::Broadcast(const FunctionSpec& source, BroadcastCallback callback) {
    NAPA_ASSERT(_instance != nullptr, "Node zone is not initialized.");
    _broadcast(source, callback, _foregroundTaskRunner);
}

void NodeZone::Execute(const FunctionSpec& spec, ExecuteCallback callback) {
    NAPA_ASSERT(_instance != nullptr, "Node zone is not initialized.");
    _execute(spec, callback, _foregroundTaskRunner);
}
