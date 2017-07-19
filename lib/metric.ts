// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

let binding = require('./binding');

export enum MetricType {
    Number = 0,
    Rate,
    Percentile,
}

export interface Metric {
    readonly section: string;
    readonly name: string;

    set(value: number, dimensions?: string[]): void;
    increment(dimensions?: string[]): void;
    decrement(dimensions?: string[]): void;
}

/// <summary> A cache for metric wraps. </summary>
var _metricsCache: { [key: string]: Metric } = {};

export function get(section: string, name: string, type: MetricType, dimensions: string[] = []) : Metric {
    let key: string = (section ? section : "") + "\\" + (name ? name : "");

    // Check cache first
    let metric: Metric = _metricsCache[key];
    if (metric) {
        return metric;
    }

    // Add to cache
    let metricWrap: any = new binding.MetricWrap(section, name, type, dimensions);
    metricWrap.section = section;
    metricWrap.name = name;
    _metricsCache[key] = metricWrap;

    return metricWrap;
}
