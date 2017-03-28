var binding = process.binding('fs');
var fs = exports;

fs.readFileSync = function(path, options) {
    return binding.readFileSync(path);
}

fs.writeFileSync = function(path, data, options) {
    return binding.writeFileSync(path, data);
}

fs.mkdirSync = function(path, mode) {
    return binding.mkdirSync(path);
}

fs.existsSync = function(path) {
    return binding.existsSync(path);
}

fs.runTest = function() {
    return 'fs.runTest';
}