"use strict";
var childProcess = require('child_process');
var fs = require('fs');
var os = require("os");
var path = require("path");

// Remove first two parameters which are node executable and this javascript file.
var params = process.argv.slice(2);
var clean = params.indexOf('clean') != -1 ? true : false;
// If debug/release is not specific, relase is the default.
var buildType = params.indexOf('debug') != -1 ? "debug" : "release";

var napaRoot = path.join(__dirname, "..");
process.chdir(napaRoot);

if (clean) {
    console.log('Cleaning up build files.');

    // Remove build folder
    var buildDir = path.join(napaRoot, "build");
    console.log(`Removing ${buildDir}.`);
    removeDirectory(buildDir);
    
    // Remove bin folder
    var binDir = path.join(napaRoot, "bin");
    console.log(`Removing ${binDir}.`);
    removeDirectory(binDir);

    return 0;
}

// Node.Js version
// This version of node builts successfully as a dll (not all version do).
var nodeVersion = "v6.10.3";
var nodeCloneRoot = path.join(napaRoot, "build/node-" + nodeVersion);

// Refer to https://github.com/nodejs/node/blob/master/BUILDING.md#windows-1 for prerequisites.
childProcess.execSync(`git clone --branch ${nodeVersion} https://github.com/nodejs/node.git ${nodeCloneRoot}`, {
    stdio: 'inherit'
});

if (os.platform() === "win32") {
    // Build node shared library to extract intermediate v8 static libraries to build napa.dll.
    childProcess.execSync(`vcbuild.bat x64 ${buildType} nosign dll`, {
        cwd: nodeCloneRoot,
        stdio: 'inherit'
    });
}
// TODO (asib): support other platforms

// Create platform specific build files using cmake.
childProcess.execSync(`cmake -A x64 -DNODE_ROOT=${nodeCloneRoot} -DCMAKE_BUILD_TYPE=${buildType} ..`, {
    cwd: path.join(napaRoot, "build"),
    stdio: 'inherit'
});

// Build.
childProcess.execSync(`cmake --build . --config ${buildType}`, {
    cwd: path.join(napaRoot, "build"),
    stdio: 'inherit'
});

// Helper function for removing all files from a directory.
function removeDirectory(dirPath) {
    if (fs.existsSync(dirPath)) {
        fs.readdirSync(dirPath).forEach((file, index) => {
            var currentPath = dirPath + "/" + file;
            if (fs.lstatSync(currentPath).isDirectory()) {
                // Recursive call
                removeDirectory(currentPath);
            } 
            else {
                // delete a file
                fs.unlinkSync(currentPath);
            }
        });

        // Remove the empty directory.
        fs.rmdirSync(dirPath);
    }
}