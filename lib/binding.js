// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

function warn(messages) {
    messages.forEach(function(message) {
        console.log('\x1b[1m\x1b[33m', message, '\x1b[0m');
    });
}

function fail(messages) {
    warn(messages);
    throw new Error(messages.join('\n'));
}

function checkNodeVersion() {
    if (process.version > 'v8.4.0' || process.version.indexOf('.') > 2) {
        warn([
            'Thanks for using Napa.js.',
            'There is a compatibility issue on Node v8.5.0 and above.',
            'The flag "--noincremental-marking" is set to disable V8 incremental marking as a workaround.',
            'We are working with Node.js team on a fix in newer Node versions.',
        ]);
        require('v8').setFlagsFromString('--noincremental-marking');
    }
    else if (process.version < 'v4.5') {
        fail(['Napa.js is not supported on Node version lower than v4.5']);
    }
}

if (typeof __in_napa === 'undefined') {
    checkNodeVersion();
    module.exports = require('../bin/napa-binding');
} else {
    module.exports = process.binding('napa-binding');
}
