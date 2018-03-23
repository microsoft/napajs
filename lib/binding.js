// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

function checkNodeVersion() {
    var semver = require('semver');
    var currentNodeVersion = semver.coerce(process.version);
    if (semver.gt(currentNodeVersion, 'v8.4.0')) {
        var log = require('npmlog');
        log.warn('napajs binding', 'Thanks for using Napa.js.');
        log.warn('napajs binding', 'There is a compatibility issue on Node v8.5.0 and above.');
        log.warn('napajs binding', 'The flag "--noincremental-marking" is set to disable V8 incremental marking as a workaround.');
        log.warn('napajs binding', 'We are working with Node.js team on a fix in newer Node versions.');

        require('v8').setFlagsFromString('--noincremental-marking');
    } else if (semver.lt(currentNodeVersion, 'v4.5.0')) {
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
module.exports = require('../bin/napa-binding');

