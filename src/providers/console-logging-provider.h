#pragma once

#include <napa/providers/logging.h>

#include <stdio.h>

namespace napa {
namespace providers {

    /// <summary> A logging provider that logs to the standard console. </summary>
    class ConsoleLoggingProvider : public LoggingProvider {
    public:

        virtual void LogMessage(
            const char* section,
            Verboseness level,
            const char* traceId,
            const char* message) override {
            printf("[%s] %s\n", section, message);
        }

        virtual bool IsLogEnabled(const char* section, Verboseness level) override {
            return true;
        }

        virtual void Destroy() override {
            // Don't actually delete. We're a lifetime process object.
        }
    };

}
}
