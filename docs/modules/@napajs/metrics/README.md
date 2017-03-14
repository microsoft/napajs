# @napajs/metrics: Accessing Metric APIs

## Introduction
`@napajs/metrics` is a facade module to exposing API to access metrics. Like `@najajs/logger`, metrics module allow user to set metric provider, thus user don't need to update code to switch between different metric implementation.



## Setup
```typescript
import metrics = require('@napajs/metrics');

class MyMetric implements metrics.Metric {
    public constructor(section: string, 
                     name: string, 
                     type: metrics.MetricType, 
                     dimensionNames: string[]) {
        this.section = section;
        this.name = name;
    }

    public increment(dimensionValues: string[] = []) {
        var key = JSON.stringify(dimensionValues);
        if (values[key] == null)
            values[key] = 0;
        values[key]--;
    }

    public decrement(dimensionValues: string[] = []) {
        // User logic.
        var key = JSON.stringify(dimensionValues);
        if (values[key] == null)
            values[key] = 0;
        values[key]++;
    }

    public set(value: number, dimensionValues: string[] = []) {
        var key = JSON.stringify(dimensionValues);
        values[key] = value;
    }

    private section: string;
    private name: string;
    private values: {[key: string]: number} = {}
}

class MyMetricProvider implements metrics.MetricProvider {
    public getMetric(section: string, 
                     name: string, 
                     type: metrics.MetricType, 
                     dimensionNames: string[]): metrics.Metric {
        return new MyMetric(section, name, type, dimensionNames);
    }
}

metrics.setProvider(new MyMetricProvider());
```
You can also use metric provider from `napajs`.
Please note when it's not running under Napa container. `getMetricProvider` will return undefined and metric provider will be set to NoOpMetricProvider.

```typescript
import metrics = require('@napajs/metrics');
import napa = require('napajs');

metrics.setProvider(napa.getMetricProvider());
```

## Usage
```typescript
// Example: Metrics for number.
{
    let count: metrics.Metric = 
        metrics.get("Example Code",      // Metric section name.
                    "Total QPS",         // Metric name.
                    metrics.Number,      // Set type to Number.
                    []                   // No dimensions.
                    )
    count.increment();
    count.decrement();
    
    count.set(100);
}

// Example: Metrics for rate.
{
    let rate: metrics.Metric = 
        metrics.get("Example Code",      // Metric section name.
                    "Total QPS",         // Metric name.
                    metrics.Rate,        // Set type to Rate.
                    ['Entry Point Name'] // 1 dimension.
                    )
    rate.increment('Example Entrypoint');
    rate.decrement('Example Entrypoint');
}

// Example: Metrics for recording a timespan.
{
    let latency: metrics.Metric = 
        metrics.get("Example Code",        // Metric section name.
                    "Process Latency",        // Metric name.
                    metrics.Percentile,       // Metric type.
                    ["Function Name"]         // Function name as the only 1 dimension.
                    );

    metrics.recordElapse(latency, () => {
            func1();
        },
        ["Call func1"]);

    metrics.recordElapse(latency, () => {
            func2();
        },
        ["Call func2"]);
}

// Example: using metrics.TimePoint
{
    // Another way of setting metrics.
    let latency: metrics.Metric = 
        metrics.get("Example Code",        // Metric section name.
                    "Process Latency",        // Metric name.
                    metrics.Percentile,       // Metric type.
                    ["Function Name"]         // Function name as the only 1 dimension.
                    );

    var start: metrics.TimePoint = metrics.now();
    func3();
    var durationInMS: number = metrics.elapseSince(start);
    latency.set(durationInMS, ["Call func3"]);

    // TODO: do we support converting TimePoint to Date? (std::chrono support convert time_point to std::time_t)
    // TODO: do we also support converting Date to TimePoint?
}
```

## Interfaces

```typescript
enum MetricType {
    Number,
    Rate,
    Percentile
}

interface Metric {
    set(value: number, dimensionValues: string[] = []): void;

    increment(dimensionValues: string[] = []): void;

    decrement(dimensionValues: string[] = []): void;
}

```