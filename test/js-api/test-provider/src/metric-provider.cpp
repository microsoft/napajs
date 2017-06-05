#include "metric-provider.h"

using namespace napa::providers;

TestMetric::TestMetric() : _value(0) {
}

bool TestMetric::Set(int64_t value, size_t, const char*[]) {
    _value = value;
    return true;
}

bool TestMetric::Increment(uint64_t value, size_t, const char*[]) {
    _value += value;
    return true;
}

bool TestMetric::Decrement(uint64_t value, size_t, const char*[]) {
    _value -= value;
    return true;
}

void TestMetric::Destroy() {
    delete this;
}

int64_t TestMetric::GetValue() const {
    return _value;
}


TestMetricProvider& TestMetricProvider::GetInstance() {
    static TestMetricProvider instance;

    return instance;
}

Metric* TestMetricProvider::GetMetric(const char*, const char*, MetricType, size_t, const char*[]) {
    _metrics.emplace_back(TestMetric());
    return &_metrics.back();
}

const TestMetric* TestMetricProvider::GetLastMetric() const {
    return &_metrics.back();
}

void TestMetricProvider::Destroy() {
    // Singleton
}

// Export a function for creating the test metric provider
EXTERN_C NAPA_API MetricProvider* CreateMetricProvider() {
    return &TestMetricProvider::GetInstance();
}
