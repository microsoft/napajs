// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

/// <summary> Utility function to generate object for testing. </summary>

export function generateString(length: number): string {
    return Array(length).join('x');
}

export function generateObject(keys: number, depth: number, valueType: string = "string", valueLength = 7) {
    let object: any = {};
    for (let i = 0; i < keys; ++i) {
        let key = `key${i}`;
        let value: any = null;
        if (depth > 1) {
            value = generateObject(keys, depth - 1, valueType, valueLength);
        } else if (valueType === 'string') {
            // We try to make each string different.
            value = generateString(valueLength - 1) + (depth * keys + i);
        } else if (valueType === 'number') {
            value = i;
        } else if (valueType === 'boolean') {
            value = i % 2 == 0;
        }
        object[key] = value;
    }
    return object;
}

export function timeDiffInMs(diff:[ number, number ]): number {
    return (diff[0] * 1e9 + diff[1]) / 1e6;
}

export function formatTimeDiff(diff: number | [number, number], printUnit: boolean = false): string {
    if (Array.isArray(diff)) {
        diff = timeDiffInMs(diff);
    }
    let message = diff.toFixed(2);
    if (printUnit) {
        message += "ms"
    }
    return message;
}

export function formatRatio(dividend: number, divider: number): string {
    return "(" + (dividend / divider).toFixed(2) + "x)";
}