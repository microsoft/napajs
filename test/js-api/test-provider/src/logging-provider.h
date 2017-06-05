#pragma once

#include <napa/providers/logging.h>

#include <string>

struct LogEntry {
    std::string section;
    napa::providers::LoggingProvider::Verboseness level;
    std::string traceId;
    std::string file;
    int line;
    std::string message;
};

class TestLoggingProvider : public napa::providers::LoggingProvider {
public:

    NAPA_API static TestLoggingProvider& GetInstance();

    NAPA_API const LogEntry& GetLastLog() const;

    void LogMessage(
        const char* section,
        Verboseness level,
        const char* traceId,
        const char* file,
        int line,
        const char* message) override;

    bool IsLogEnabled(const char *, Verboseness) override;

    void Destroy() override;

private:

    LogEntry _lastLog;

    TestLoggingProvider() = default;
};
