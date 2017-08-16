// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

let addon = require('../bin/addon');

export declare class PlusNumber {
    public add(value: number): number;
}

export function createPlusNumber(value: number = 0.0): PlusNumber {
    return addon.createPlusNumber(value);
}
