// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

var path = require('path');
var childProcess = require('child_process');

try {
    childProcess.execFileSync(
        path.join(__dirname, 'build/test/', process.platform === 'win32' ? 'napa-unittest.exe' : 'napa-unittest'),
        [],
        { cwd : path.join(__dirname, 'build/test'), stdio : 'inherit' });
} catch (err) {
    process.exit(1); // Error
}
