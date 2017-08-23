// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "settings-parser.h"

#include <napa/log.h>

// Open source header only library for argument parsing.
// https://github.com/Taywee/args
#include <args/args.hxx>

using namespace napa;
using namespace napa::settings;


bool settings::Parse(const std::vector<std::string>& args, PlatformSettings& settings) {
    args::ArgumentParser parser("platform settings parser");

    args::ValueFlag<std::string> loggingProvider(parser, "loggingProvider", "logging provider", {"loggingProvider"});
    args::ValueFlag<std::string> metricProvider(parser, "metricProvider", "metric provider", {"metricProvider"});

    try {
        parser.ParseArgs(args);
    } catch (const std::exception& ex) {
        LOG_ERROR("Settings", "Failed to parse platform settings. Error: %s", ex.what());
        return false;
    }

    if (loggingProvider) {
        settings.loggingProvider = loggingProvider.Get();
    }

    if (metricProvider) {
        settings.metricProvider = metricProvider.Get();
    }

    return true;
}

bool settings::Parse(const std::vector<std::string>& args, ZoneSettings& settings) {
    args::ArgumentParser parser("zone settings parser");

    args::ValueFlag<uint32_t> workers(parser, "workers", "number of zone workers", {"workers"});
    args::ValueFlag<uint32_t> maxOldSpaceSize(parser, "maxOldSpaceSize", "max old space size in MB", {"maxOldSpaceSize"});
    args::ValueFlag<uint32_t> maxSemiSpaceSize(parser, "maxSemiSpaceSize", "max semi space size in MB", {"maxSemiSpaceSize"});
    args::ValueFlag<uint32_t> maxExecutableSize(parser, "maxExecutableSize", "max executable size in MB", {"maxExecutableSize"});
    args::ValueFlag<uint32_t> maxStackSize(parser, "maxStackSize", "max isolate stack size in bytes", {"maxStackSize"});

    try {
        parser.ParseArgs(args);
    } catch (const std::exception& ex) {
        LOG_ERROR("Settings", "Failed to parse zone settings. Error: %s", ex.what());
        return false;
    }

    if (workers) {
        NAPA_ASSERT(workers.Get() > 0, "The number of workers must be greater than 0");
        settings.workers = workers.Get();
    }

    if (maxOldSpaceSize) {
        settings.maxOldSpaceSize = maxOldSpaceSize.Get();
    }

    if (maxSemiSpaceSize) {
        settings.maxSemiSpaceSize = maxSemiSpaceSize.Get();
    }

    if (maxExecutableSize) {
        settings.maxExecutableSize = maxExecutableSize.Get();
    }

    if (maxStackSize) {
        NAPA_ASSERT(maxStackSize.Get() > 0, "The maximum allowed stack size must be greater than 0");
        settings.maxStackSize = maxStackSize.Get();
    }

    return true;
}