// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

function failCheck(messages) {
    messages.forEach(function(message) {
        console.log('\x1b[1m\x1b[33m', message, '\x1b[0m');
    });
    throw new Error(messages.join(' '));
}

function checkNodeVersion() {
    if (process.version > 'v8.4.0' || process.version.indexOf('.') > 2) {
        failCheck([
            'Thanks for using Napa.js.',
            'There is a compatibility issue on Node v8.5.0 and above.',
            'We are working with Node.js team on a fix in newer Node versions.',
            'Please use Node with version v8.4.0 or lower.'
        ]);
    }
    else if (process.version < 'v4.5') {
        failCheck(['Napa.js is not supported on Node version lower than v4.5']);
    }
}

if (typeof __in_napa === 'undefined') {
    checkNodeVersion();
    module.exports = require('../bin/napa-binding');
} else {
    module.exports = process.binding('napa-binding');
}
