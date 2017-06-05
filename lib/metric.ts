let binding = require('./binding');

export enum MetricType {
    Number = 0,
    Rate,
    Percentile,
}

export interface Metric {
    section: string;
    name: string;

    set(value: number, dimensions?: string[]): void;
    increment(value: number, dimensions?: string[]): void;
    decrement(value: number, dimensions?: string[]): void;
}

/// <summary> A cache for metric wraps. </summary>
var _metricsCache: { [key: string]: Metric } = {};

export function get(section: string, name: string, type: MetricType, dimensions: string[] = []) : Metric {
    let key: string = (section ? section : "") + "_" + (name ? name : "");

    // Check cache first
    let metricWrap: Metric = _metricsCache[key];
    if (metricWrap) {
        return metricWrap;
    }

    // Add to cache
    metricWrap = new binding.MetricWrap(section, name, type, dimensions);
    metricWrap.section = section;
    metricWrap.name = name;
    _metricsCache[key] = metricWrap;

    return metricWrap;
}
