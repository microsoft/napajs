// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

var path = require('path');
var childProcess = require('child_process');

try {
    childProcess.execFileSync(
        path.join(__dirname, '../../bin/', process.platform === 'win32'? 'library-test.exe': 'library-test'),
        [],
        {
            cwd: path.join(__dirname, '../../bin'),
            stdio: 'inherit'
        }
    );
}
catch(err) {
    process.exit(1); // Error
}
