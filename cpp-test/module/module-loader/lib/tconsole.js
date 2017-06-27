var tconsole = exports;

for (var prop in console) {
    tconsole[prop] = console[prop];
}

tconsole.runTest = function() {
    return 'tconsole.runTest';
}