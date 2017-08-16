// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/providers/logging.h>

namespace napa {
namespace providers {

    /// <summary> A logging provider that does nothing. </summary>
    class NopLoggingProvider : public LoggingProvider {
    public:
        virtual void LogMessage(const char* section,
                                Verboseness level,
                                const char* traceId,
                                const char* file,
                                int line,
                                const char* message) override {}

        virtual bool IsLogEnabled(const char* section, Verboseness level) override { return false; }

        virtual void Destroy() override {
            // Don't actually delete. We're a lifetime process object.
        }
    };
}
}
