// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

const napa = require("napajs");
const assert = require("assert");

const MILLION = 1024 * 1024;

// Change this value to control number of napa workers initialized.
const NUMBER_OF_WORKERS = 4;

// Create a napa zone with number_of_workers napa workers.
let zone = napa.zone.create('zone', { workers: NUMBER_OF_WORKERS });

// Swap elements with index 'left' and 'right' in a TypedArray.
function swap(typedArray, left, right) {
    let temp = typedArray[left];
    typedArray[left] = typedArray[right];
    typedArray[right] = temp;
}

function partition(typedArray, left, right) {
    let pi = left - 1;
    let pivot = typedArray[right];

    for (let index = left; index <= right; index++) {
        if (typedArray[index] <= pivot) {
            swap(typedArray, ++pi, index);
        }
    }

    return pi;
}

function quickSort(typedArray, left, right) {
    if (right <= left) return;

    let pi = partition(typedArray, left, right);
    quickSort(typedArray, left, pi - 1);
    quickSort(typedArray, pi + 1, right);
}

// A parallel version of quicksort by napa.js.
// After partition, if any part (left or right) is longer than parallelLength,
// it will recursively execute parallelQuickSort, otherwise, it will execute quickSort.
function parallelQuickSort(typedArray, left, right, parallelLength) {
    if (right <= left) return;

    let pi = global.partition(typedArray, left, right);

    let promises = [];

    if (pi - left >= parallelLength) {
        promises.push(global.napa.zone.get('zone').execute('', 'parallelQuickSort', [typedArray, left, pi - 1, parallelLength]));
    } else {
        promises.push(global.napa.zone.get('zone').execute('', 'quickSort', [typedArray, left, pi - 1]));
    }

    if (right - pi >= parallelLength) {
        promises.push(global.napa.zone.get('zone').execute('', 'parallelQuickSort', [typedArray, pi + 1, right, parallelLength]));
    } else {
        promises.push(global.napa.zone.get('zone').execute('', 'quickSort', [typedArray, pi + 1, right]));
    }

    return Promise.all(promises).then(values => {return true;});
}

function run(length) {
    let sab1 = new SharedArrayBuffer(length * 8);
    let ta1 = new Float64Array(sab1);
    let sab2 = new SharedArrayBuffer(length * 8);
    let ta2 = new Float64Array(sab2);

    // Initialize the 2 TypedArrays with the same random numbers.
    for (let i = 0; i < length; i++) {
        let temp = Math.random();
        ta1[i] = temp;
        ta2[i] = temp;
    }

    // Execute quickSort.
    let start1 = Date.now();
    quickSort(ta1, 0, length - 1);
    console.log('quickSort:\t\tIt took (', Date.now() - start1, ') MS to sort an array with ', length, ' elements.');
    // Assert the TypedArray has been sorted in ascending order.
    for (let i = 0; i < length - 1; i++) {
        assert(ta1[i] <= ta1[i + 1], 'the array is not in ascending order after quicksort.')
    }

    // Execute parallelQuickSort.
    let parallelLength = length / NUMBER_OF_WORKERS;
    let start2 = Date.now();
    return zone.execute(parallelQuickSort, [ta2, 0, length - 1, parallelLength]).then(result => {
        console.log('parallelQuickSort:\tIt took (', Date.now() - start2, ') MS to sort an array with ', length, ' elements.');

        // Assert the TypedArray has been sorted in ascending order.
        for (let i = 0; i < length - 1; i++) {
            assert(ta2[i] <= ta2[i + 1], 'the array is not in ascending order after quicksort.')
        }
    });
}

// Bootstrap zone workers.
zone.broadcast('napa = require("napajs");');
zone.broadcast(swap.toString());
zone.broadcast(partition.toString());
zone.broadcast(quickSort.toString());
zone.broadcast(parallelQuickSort.toString());

console.log();

// Run with length of 4 millions.
run(4 * MILLION);
