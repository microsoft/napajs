var path = require('path');
var childProcess = require('child_process');

childProcess.execFileSync(path.join(__dirname, 'bin/napa-unittest.exe'), [], { stdio: 'inherit' });