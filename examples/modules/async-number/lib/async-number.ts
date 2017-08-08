// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

var addon = require('../bin/addon');

export function increase(value: number, callback: (now: number) => void) {
    return addon.increase(value, callback);
}

export function increaseSync(value: number, callback: (now: number) => void) {
    return addon.increaseSync(value, callback);
}

export function now(): string {
    return addon.now();
}
