#pragma once

#include <napa/providers/metric.h>

#include <string>
#include <vector>

class TestMetric : public napa::providers::Metric {
public:

    explicit TestMetric();

    bool Set(int64_t value, size_t, const char*[]) override;

    bool Increment(uint64_t value, size_t, const char*[]) override;

    bool Decrement(uint64_t value, size_t, const char*[]) override;

    void Destroy() override;

    NAPA_API int64_t GetValue() const;

private:
    int64_t _value;
};

class TestMetricProvider : public napa::providers::MetricProvider {
public:

    NAPA_API static TestMetricProvider& GetInstance();

    NAPA_API const TestMetric* GetLastMetric() const;

    napa::providers::Metric* GetMetric(
        const char* section,
        const char* name,
        napa::providers::MetricType type,
        size_t dimensions,
        const char* dimensionNames[]) override;

    void Destroy() override;

private:
    std::vector<TestMetric> _metrics;
    
    TestMetricProvider() = default;
};
