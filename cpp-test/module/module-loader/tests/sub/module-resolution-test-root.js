var assert = require('assert');

//
// Test require with path information
//
module.exports = function () {
    var paths = [
        "fs",
        "./sub1/file1.js",
        "./sub1/file1",
        "../sub/sub1/file1.js",
        // From module root via relative path.
        "../../sample.napa",
        "../../sample",
        // from .\node_modules to parents' recursively.
        "file2.js",
        "file2"
    ];
    
    for (i = 0; i < paths.length; i++) {
        var file = require(paths[i]);
        if (file == null) {
            console.log("Failed to resolve module at " + path[i]);
            return false;
        }
    }
    
    return true;
};

var path = require('path');

assert.equal(__filename, path.resolve(__dirname, "module-resolution-test-root.js"));

assert.equal(__dirname, path.resolve("tests\\sub"));

assert.equal(module.id, path.resolve(__dirname, "module-resolution-test-root.js"));