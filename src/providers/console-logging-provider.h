#pragma once

#include <napa/providers/logging.h>

#include <stdarg.h>

namespace napa {
namespace providers {

    /// <summary> A logging provider that logs to the standard console. </summary>
    class ConsoleLoggingProvider : public LoggingProvider {
    public:

        virtual void Log(const char* section,
                         Verboseness level,
                         const char* traceId,
                         const char* title,
                         const char* format, ...) override {
            va_list vl;
            va_start(vl, format);
            vfprintf(stdout, format, vl);
            va_end(vl);
        }

        virtual bool IsLogEnabled(const char* section, Verboseness level, const char* title) override {
            return true;
        }

        virtual void Destroy() override {
            // Don't actually delete. We're a lifetime process object.
        }
    };

}
}
