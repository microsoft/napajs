// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

var napa = require("napajs");

// Change this value to control number of napa workers initialized.
const NUMBER_OF_WORKERS = 4;

// Create a napa zone with number_of_workers napa workers.
var zone = napa.zone.create('zone', { workers: NUMBER_OF_WORKERS });

/*
Fibonacci sequence starts with 0 and 1,
which we'll think of as the zeroth and first Fibonacci numbers,
and each succeeding number is the sum of the two preceding Fibonacci numbers.
Thus, the second number is 0 + 1 = 1.
And to get the third Fibonacci number, we'd sum the first (1) and the second (1) to get 2.
And the fourth is the sum of the second (1) and the third (2), which is 3.
And so on.

n:              |   0   1   2   3   4   5   6   7   8   9   10  11  ...
-------------------------------------------------------------------------
NTH Fibonacci:  |   0   1   1   2   3   5   8   13  21  34  55  89  ...
*/

function fibonacci(n) {
    if (n <= 1) {
        return n;
    }

    var p1 = zone.execute("", "fibonacci", [n - 1]);
    var p2 = zone.execute("", "fibonacci", [n - 2]);

    // Returning promise to avoid blocking each worker.
    return Promise.all([p1, p2]).then(([result1, result2]) => {
        return result1.value + result2.value;
    });
}

function run(n) {
    var start = Date.now();

    return zone.execute('', "fibonacci", [n])
        .then(result => {
            printResult(n, result.value, Date.now() - start);
            return result.value;
        });
}

function printResult(nth, fibonacci, ms) {
    console.log('\t' + nth
          + '\t' + fibonacci
          + '\t\t' + NUMBER_OF_WORKERS
          + '\t\t' + ms);
}

console.log();
console.log('\tNth\tFibonacci\t# of workers\tlatency in MS');
console.log('\t-----------------------------------------------------------');

// Broadcast declaration of 'napa' and 'zone' to napa workers.
zone.broadcast(' \
    var napa = require("napajs"); \
    var zone = napa.zone.get("zone"); \
');
// Broadcast function declaration of 'fibonacci' to napa workers.
zone.broadcast(fibonacci.toString());

// Run fibonacci eavluation in sequence.
run(10)
.then(result => { run(11)
.then(result => { run(12)
.then(result => { run(13)
.then(result => { run(14)
.then(result => { run(15)
.then(result => { run(16)
}) }) }) }) }) })
