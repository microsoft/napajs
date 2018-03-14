// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "worker-context.h"

#include <napa/log.h>
#include <platform/process.h>
#include <platform/thread-local.h>
#include <utils/debug.h>

#include <array>
#include <sstream>
#include <string>
#include <thread>

using namespace napa;
using namespace napa::zone;

namespace {
    tls::ThreadLocal<std::array<
        void*,
        static_cast<size_t>(WorkerContextItem::END_OF_WORKER_CONTEXT_ITEM)>> items;
}

void WorkerContext::Init() {
    items.Install();
    items->fill(nullptr);
}

void* WorkerContext::Get(WorkerContextItem item) {
    std::ostringstream ss;
    ss << "Get item " << (uint32_t)item;
    ss << " from " << reinterpret_cast<std::uintptr_t>(&items);
    ss << " of thread " << std::this_thread::get_id();
    NAPA_DEBUG("WorkerContext", ss.str().c_str());
    NAPA_ASSERT(item < WorkerContextItem::END_OF_WORKER_CONTEXT_ITEM, "Invalid WorkerContextItem");
    return (*items)[static_cast<size_t>(item)];
}

void WorkerContext::Set(WorkerContextItem item, void* data) {
    std::ostringstream ss;
    ss << "Set item " << (uint32_t)item;
    ss << " from " <<  reinterpret_cast<std::uintptr_t>(&items);
    ss << " of thread " << std::this_thread::get_id();
    NAPA_DEBUG("WorkerContext", ss.str().c_str());
    NAPA_ASSERT(item < WorkerContextItem::END_OF_WORKER_CONTEXT_ITEM, "Invalid WorkerContextItem");
    (*items)[static_cast<size_t>(item)] = data;
}