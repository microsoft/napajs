#pragma once

#include <napa/providers/logging.h>


namespace napa {
namespace providers {

    /// <summary> A logging provider that does nothing. </summary>
    class NopLoggingProvider : public LoggingProvider {
    public:

        virtual void Log(const char* section,
            Verboseness level,
            const char* traceId,
            const char* format, ...) override {}

        virtual bool IsLogEnabled(const char* section, Verboseness level) override {
            return false;
        }

        virtual void Destroy() override {
            // Don't actually delete. We're a lifetime process object.
        }

    };

}
}
