// clang-format all source files in napa.js

var _getAllFilesFromFolder = function(dir) {

    var filesystem = require("fs");
    var results = [];

    filesystem.readdirSync(dir).forEach(function(file) {

        file = dir + '/' + file;
        var stat = filesystem.statSync(file);

        if (stat && stat.isDirectory()) {
            results = results.concat(_getAllFilesFromFolder(file))
        } else
            results.push(file);

    });

    return results;

};

var clang_format_folder = function(name) {

    var path = require('path');
    _getAllFilesFromFolder(path.join(__dirname, name))
        .forEach(function(file) {

            if (file.endsWith('.cpp') || file.endsWith('.h') || file.endsWith('.js') || file.endsWith('.ts')) {
                console.log(file);
                require('child_process').execSync('clang-format -i ' + file);
            }
        });
};

clang_format_folder('benchmark');
clang_format_folder('examples');
clang_format_folder('inc');
clang_format_folder('lib');
clang_format_folder('node');
clang_format_folder('src');
clang_format_folder('test');
clang_format_folder('unittest');
