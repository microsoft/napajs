// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

var napa = require("napajs");

// Change this value to control number of napa workers initialized.
const NUMBER_OF_WORKERS = 4;

// Create a napa zone with number_of_workers napa workers.
var zone = napa.zone.create('zone', { workers: NUMBER_OF_WORKERS });

/*
Estimate the value of π by using a Monte Carlo method.
Take `points` samples of random x and y values on a
[0,1][0,1] plane. Calculating the length of the diagonal
tells us whether the point lies inside, or outside a
quarter circle running from 0,1 to 1,0. The ratio of the
number of points inside to outside gives us an
approximation of π/4.
See https://en.wikipedia.org/wiki/File:Pi_30K.gif
for a visualization of how this works.
*/

function estimatePI(points) {
    var i = points;
    var inside = 0;

    while (i-- > 0) {
        var x = Math.random();
        var y = Math.random();
        if ((x * x) + (y * y) <= 1) {
            inside++;
        }
    }

    return inside / points * 4;
}

function run(points, batches) {
    var start = Date.now();

    var promises = [];
    for (var i = 0; i < batches; i++) {
        promises[i] = zone.execute(estimatePI, [points / batches]);
    }
    
    return Promise.all(promises).then(values => {
        var aggregate = 0;
        values.forEach(result => aggregate += result.value);
        printResult(points, batches, aggregate / batches, Date.now() - start);
    });
}

function printResult(points, batches, pi, ms) {
    console.log('\t' + points
          + '\t\t' + batches
          + '\t\t' + NUMBER_OF_WORKERS
          + '\t\t' + ms
          + '\t\t' + pi.toPrecision(7)
          + '\t' + Math.abs(pi - Math.PI).toPrecision(7));
}

console.log();
console.log('\t# of points\t# of batches\t# of workers\tlatency in MS\testimated π\tdeviation');
console.log('\t---------------------------------------------------------------------------------------');

// Run with different # of points and batches in sequence.
run(4000000, 1)
.then(result => { run(4000000, 2)
.then(result => { run(4000000, 4)
.then(result => { run(4000000, 8)
}) }) })
