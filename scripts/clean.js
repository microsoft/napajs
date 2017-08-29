// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

"use strict";
var fs = require('fs');
var path = require("path");

exports.clean = function() {
    console.log('Cleaning up build files.');
    
    // Remove build folder
    var buildDir = path.join(__dirname, "../build");
    console.log(`Removing ${buildDir}.`);
    removeDirectory(buildDir);
    
    // Remove bin folder
    var binDir = path.join(__dirname, "../bin");
    console.log(`Removing ${binDir}.`);
    removeDirectory(binDir);
}

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
