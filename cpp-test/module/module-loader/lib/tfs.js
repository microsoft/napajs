var binding = process.binding('fs');
var tfs = exports;

for (var prop in binding) {
    tfs[prop] = binding[prop];
}

tfs.runTest = function() {
    return 'tfs.runTest';
}