// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/providers/metric.h>

namespace napa {
namespace providers {

    ///<summary> A no-operation instance of a Metric. </summary>
    class NopMetric : public Metric {
    public:
        bool Set(int64_t, size_t, const char* []) override { return true; }

        bool Increment(uint64_t, size_t, const char* []) override { return true; }

        bool Decrement(uint64_t, size_t, const char* []) override { return true; }

        void Destroy() override {
            // Don't actually delete. We're a lifetime process object.
        }
    };

    ///<summary> A no-operation instance of a MetricProvider.</summary>
    class NopMetricProvider : public MetricProvider {
    public:
        NopMetricProvider() : _defaultMetric(new NopMetric()) {}

        Metric* GetMetric(const char*, const char*, MetricType, size_t, const char**) override {
            return _defaultMetric;
        }

        virtual void Destroy() override {
            // Don't actually delete. We're a lifetime process object.
        }

    private:
        Metric* _defaultMetric;
    };
}
}
