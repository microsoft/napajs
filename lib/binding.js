// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

function checkNodeVersion() {
    var semver = require('semver');
    var currentNodeVersion = semver.coerce(process.version);
    if (semver.lt(currentNodeVersion, 'v4.5.0')) {
        var errorMessage = 'Napa.js is not supported on Node version lower than v4.5';
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

