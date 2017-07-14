var assert = require('assert');

function run() {
    // Core module
    assert(require.resolve('fs'), 'fs');

    // Full path with extension
    assert(require.resolve(__dirname + '/sub-folder/file.js'), __dirname + '/sub-folder/file.js');

    // Full path without extension
    assert(require.resolve(__dirname + '/sub-folder/file'), __dirname + '/sub-folder/file.js');

    // Relative path with extension
    assert(require.resolve('./sub-folder/file.js'), __dirname + '/sub-folder/file.js');

    // Relative path without extension
    assert(require.resolve('./sub-folder/file'), __dirname + '/sub-folder/file.js');

    // Relative path with non normalized path
    assert(require.resolve('./sub-folder/.././sub-folder/file.js'), __dirname + '/sub-folder/file.js');

    // Relative path without extension to napa addon
    assert(require.resolve('./addon/build/simple-napa-addon'), __dirname + '/addon/build/simple-napa-addon.napa');

    // From node_modules with extension
    assert(require.resolve('file.js'), __dirname + '/node_modules/file.js');

    // From node_modules without extension
    assert(require.resolve('file'), __dirname + '/node_modules/file.js');

    // TODO: Fix this issue: require.resolve should throw an exception if file doesn't exist.
    // Resolving non-existing file should throw
    // assert.throws(() => { 
    //     require.resolve('./sub-folder/non-existing-file.js');
    // });
}

exports.run = run;
