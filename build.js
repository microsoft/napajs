// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

// Distinguish between running this file as a script or loading it as a module.
if (module.parent) {
    // Loaded as a module - 'require('./build.js')
    exports.paths = require('./scripts/paths');
}
else {
    // Loaded as a script - 'node build.js'

    // Remove first two parameters which are node executable and this javascript file.
    // Transform all parameters to lowercase for easier handling.
    let params = process.argv.slice(2).map((item) => item.toLowerCase());

    // Specify wheater cleanup is needed.
    if (params.indexOf('clean') != -1) {
        require('./scripts/clean').clean();
    }
    
    // Only build if embed was specified as a parameter.
    if (params.indexOf('embed') != -1) {
        let embedded = require('./scripts/embedded');

        if (params.indexOf('debug') != -1) {
            embedded.build('debug');
        }
        else {
            embedded.build('release');
        }
    }
}
