// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

if (typeof __in_napa === 'undefined') {
    module.exports = require('../bin/napa-binding');
} else {
    module.exports = process.binding('napa-binding');
}
