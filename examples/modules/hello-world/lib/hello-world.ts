// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

var addon = require('../bin/hello');

export function hello(): string {
    return addon.hello();
}