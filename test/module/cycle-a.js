// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

exports.done = false;
var cycle_b = require('./cycle-b.js');
if (cycle_b.done == true) {
    exports.done = true;
}
