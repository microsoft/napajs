#include "logging-provider.h"

using namespace napa::providers;

TestLoggingProvider& TestLoggingProvider::GetInstance() {
    static TestLoggingProvider instance;

    return instance;
}

const LogEntry& TestLoggingProvider::GetLastLog() const {
    return _lastLog;
}

void TestLoggingProvider::LogMessage(
    const char* section,
    Verboseness level,
    const char* traceId,
    const char* file,
    int line,
    const char* message) {
    _lastLog = { section, level, traceId, file, line, message };
}

bool TestLoggingProvider::IsLogEnabled(const char *, Verboseness) {
    return true;
}

void TestLoggingProvider::Destroy() {
    // Singleton
}


// Export a function for creating the test logging provider
EXTERN_C NAPA_API LoggingProvider* CreateLoggingProvider() {
    return &TestLoggingProvider::GetInstance();
}
