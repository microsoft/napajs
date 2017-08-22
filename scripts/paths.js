// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

let path = require('path');

Object.defineProperty(exports, 'lib', {
    get: function() {
        let libPath = path.resolve(__dirname, '../inc/' + getLibraryName('napa'));
        // Output the path to stdout for cmake/gyp commands.
        process.stdout.write(libPath);
        return libPath;
    }
});

Object.defineProperty(exports, 'inc', {
    get: function() {
        let incPath = path.resolve(__dirname, '../inc');
        // Output the path to stdout for cmake/gyp commands.
        process.stdout.write(incPath);
        return incPath;
    }
});

Object.defineProperty(exports, 'third_party', {
    get: function() {
        let thirdPartyPath = path.resolve(__dirname, '../third-party');
        // Output the path to stdout for cmake/gyp commands.
        process.stdout.write(thirdPartyPath);
        return thirdPartyPath;
    }
});

// Resolve library name according to platform type.
function getLibraryName(originalName) {
    if (process.platform === "win32") {
        return originalName + '.lib';
    } else if (process.platform === 'darwin') {
        return 'lib' + originalName + '.dylib';
    } else if (process.platform === 'linux') {
        return 'lib' + originalName + '.so';
    }
    return originalName;
}
