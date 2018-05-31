// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

function checkNodeVersion() {
    const semver = require('semver');
    let currentNodeVersion = semver.coerce(process.version);
    if (semver.lt(currentNodeVersion, 'v10.2.1')) {
        let errorMessage = 'Napa.js is not supported on Node version lower than v10.2.1';
        require('npmlog').error('napajs binding', errorMessage);
        throw new Error(errorMessage);
    }
}


if (typeof __in_napa === 'undefined') {
    checkNodeVersion();
    global.__zone_id = 'node';
    global.__worker_id = 'node-main';
}
module.exports = require('../bin/napa');

