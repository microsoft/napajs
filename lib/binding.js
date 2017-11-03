// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

if (typeof __in_napa === 'undefined') {
    if (process.version > 'v8.4.0') {
        console.log('\x1b[1m\x1b[33m', 'Thanks for your adoption of Napa.', '\x1b[0m');
        console.log('\x1b[1m\x1b[33m', 'The current Node version is (', process.version, ').\x1b[0m');
        console.log('\x1b[1m\x1b[33m', 'There is a compatibility issue between Napa and Node ( v8.5.0 or later ).', '\x1b[0m');
        console.log('\x1b[1m\x1b[33m', 'Please try Napa with Node ( v8.4.0 or earlier ).', '\x1b[0m');
        console.log('\x1b[1m\x1b[33m', 'We are working with Node team to fix it ASAP.', '\x1b[0m');
        process.exit();
    }
    else if (process.version < 'v4') {
        console.log('\x1b[1m\x1b[33m', 'Thanks for your adoption of Napa.', '\x1b[0m');
        console.log('\x1b[1m\x1b[33m', 'The current Node version is (', process.version, ').\x1b[0m');
        console.log('\x1b[1m\x1b[33m', 'We recommend you to run Napa with Node between v4.x to v8.4.0', '\x1b[0m');
        process.exit();
    }

    module.exports = require('../bin/napa-binding');
} else {
    module.exports = process.binding('napa-binding');
}
