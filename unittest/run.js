var path = require('path');
var childProcess = require('child_process');

try {
    childProcess.execFileSync(
        path.join(__dirname, 'build/test/napa-unittest.exe'),
        [],
        {
            cwd: path.join(__dirname, 'build/test'),
            stdio: 'inherit'
        }
    );
}
catch(err) {
    process.exit(1); // Error
}
