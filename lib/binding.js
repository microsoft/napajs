// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

if (typeof __in_napa === 'undefined') {
    if (process.version > 'v8.4.0' || process.version.indexOf('.') > 2) {
        console.log('\x1b[1m\x1b[33m', 'Thanks for using Napa.js.', '\x1b[0m');
        console.log('\x1b[1m\x1b[33m', 'There is a compatibility issue on Node v8.5.0 and above.', '\x1b[0m');
        console.log('\x1b[1m\x1b[33m', 'We are working with Node.js team on a fix in newer Node versions.', '\x1b[0m');
        console.log('\x1b[1m\x1b[33m', 'Please use Node with version v8.4.0 or lower.', '\x1b[0m');
        process.exit();
    }
    else if (process.version < 'v4.5') {
        console.log('\x1b[1m\x1b[33m', 'Napa.js is not supported on Node version lower than v4.5', '\x1b[0m');
        process.exit();
    }

    module.exports = require('../bin/napa-binding');
} else {
    module.exports = process.binding('napa-binding');
}
