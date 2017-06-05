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
            const char* file,
            int line,
            const char* message) override {
            if (section == nullptr || section[0] == '\0') {
                printf("%s [%s:%d]\n", message, file, line);
            } else {
                printf("[%s] %s [%s:%d]\n", section, message, file, line);
            }
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
