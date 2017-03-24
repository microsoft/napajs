import * as assert from "assert";

/// <summary> Defines the possible metric types. </summary>
/// <remarks> Must be kept in sync with MetricType native definition at inc/napa/providers/metric.h. </remarks>
export enum MetricType {
    Number = 0,
    Rate = 1,
    Percentile = 2
}

/// <summary> Interface to represents a multi-dimensional metric. </summary>
export interface Metric {

    /// <summary> Sets a metric value with optional dimension values. </summary>
    set(value: number, dimensionValues?: string[]);

    /// <summary> Increments a metric value with optional dimension values. </summary>
    increment(dimensionValues?: string[]);

    /// <summary> Decrement a metric value with optional dimension values. </summary>
    decrement(dimensionValues?: string[]);
}

/// <summary> Interface to represents a metric provider. </summary>
export interface MetricProvider {

    /// <summary> Gets an N-dimensional metric. </summary>
    getMetric(section: string, name: string, type: MetricType, dimensionNames: string[]): Metric;
}

/// <summary> Gets an N-dimensional metric. </summary>
export function get(section: string, name: string, type: MetricType, dimensionNames: string[] = []) : Metric {
    var metric = _metricProvider.getMetric(section, name, type, dimensionNames);
    assert(metric, "the provider failed to return a valid metric object");

    return new MetricWrap(metric, dimensionNames.length);
}

var _metricProvider: MetricProvider = undefined;

/// <summary> Sets the underlying metric provider. </summary>
export function setProvider(provider: MetricProvider) {
    assert(provider, "the provider to set can't be null");
    _metricProvider = provider;
}

export type TimePoint = [number, number];

/// <summary> Returns the current time. </summary>
export function now() : TimePoint {
    return process.hrtime();
}

/// <summary> Returns the elapsed time in microseconds. </summary>
export function elapseSince(timePoint: TimePoint) : number {
    let diff = process.hrtime(timePoint);
    return Math.round((diff[0] * 1e9 + diff[1]) / 1000);
}

/// <summary> Set the provided metric with the elapse time of the operation in microseconds. </summary>
export function recordElapse(metric: Metric, operation: Function, dimensionValues?: string[]) : any {
    var startTime = now();
    var result = operation();
    metric.set(elapseSince(startTime), dimensionValues);
    
    return result;
}

class MetricWrap implements Metric {
    public constructor(metric: Metric, dimensionCount: Number) {
        this._metric = metric;
        this._dimensionCount = dimensionCount;
    }

    set(value: number, dimensionValues: string[] = []) {
        assert(this._dimensionCount === dimensionValues.length, "Incorrect number of dimensions");

        if (this._dimensionCount > 0) {
            this._metric.set(value, dimensionValues);
        } else {
            this._metric.set(value);
        }
    }

    increment(dimensionValues: string[] = []) {
        assert(this._dimensionCount === dimensionValues.length, "Incorrect number of dimensions");

        if (this._dimensionCount > 0) {
            this._metric.increment(dimensionValues);
        } else {
            this._metric.increment();
        }
    }

    decrement(dimensionValues: string[] = []) {
        assert(this._dimensionCount === dimensionValues.length, "Incorrect number of dimensions");

        if (this._dimensionCount > 0) {
            this._metric.decrement(dimensionValues);
        } else {
            this._metric.decrement();
        }
    }

    private _metric: Metric;
    private _dimensionCount: Number;
}

class NopMetric implements Metric {
    increment(dimensionValues?: string[]) {}
    decrement(dimensionValues?: string[]) {}
    set(value: number, dimensionValues?: string[]) {}
}

var _nopMetric = new NopMetric();

class NopProvider implements MetricProvider {
    getMetric(section: string, name: string, type: MetricType, dimensionNames: string[]): Metric {
        return _nopMetric;
    }
}

// Set NopProvider as default.
// NOTE: Set _metricProvider to a NopProvider instance after class NopProvider definition 
// to work around TypeScript issue: https://github.com/Microsoft/TypeScript/issues/14847
_metricProvider = new NopProvider();