var binding = console;
var console = exports;

console.log = function(message) {
    return binding.log(message);
}

console.runTest = function() {
    return 'console.runTest';
}