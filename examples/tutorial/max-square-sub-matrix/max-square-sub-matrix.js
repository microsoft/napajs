// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

var napa = require("napajs");

// Change this value to control number of napa workers initialized.
const NUMBER_OF_WORKERS = 4;

// Create a napa zone with number_of_workers napa workers.
var zone = napa.zone.create('zone', { workers : NUMBER_OF_WORKERS });

// Create a napa store with 'sub-matrix-size-store' as its key.
// It is used to communicate max sub binary matrix size across isolates.
var subMatrixSizeStore = napa.store.create('sub-matrix-size-store');

function run() {
    var squareMatrix = [
        [ 0, 1, 1, 0, 1, 1 ],
        [ 1, 1, 0, 1, 0, 1 ],
        [ 0, 1, 1, 1, 0, 1 ],
        [ 1, 1, 1, 1, 0, 0 ],
        [ 1, 1, 1, 1, 1, 1 ],
        [ 0, 0, 0, 0, 0, 0 ]
    ];

    // Setup all workers with functions to be executed.
    zone.broadcast(' \
        var napa = require("napajs"); \
        var zone = napa.zone.get("zone"); \
        var store = napa.store.get("sub-matrix-size-store"); \
    ');
    zone.broadcast(get_key_of_store.toString());
    zone.broadcast(max_square_sub_matrix_with_all_1s_ended_at.toString());
    zone.broadcast(max_square_sub_matrix_with_all_1s_at_layer.toString());
    zone.broadcast(max_square_sub_matrix_with_all_1s.toString());

    var start = Date.now();

    // Start to execute.
    return zone.execute('', 'max_square_sub_matrix_with_all_1s', [ squareMatrix ])
        .then(result => { print_result(squareMatrix, Date.now() - start); });
}

/*
Given a binary square matrix, find out the maximum square sub matrix with all 1s.
For example, consider the below binary matrix [example_matrix],

    0   1   1   0   1   1
    1   1   0   1   0   1
    0   1   1   1   0   1
    1   1   1   1   0   0
    1   1   1   1   1   1
    0   0   0   0   0   0

the max square sub matrix with all 1s is

    -   -   -   -   -   -
    -   -   -   -   -   -
    -   1   1   1   -   -
    -   1   1   1   -   -
    -   1   1   1   -   -
    -   -   -   -   -   -

with size = 3, and the lowest-rightest element = example_matrix[4, 3].

Notation:
    SubMatrixSize[i, j]: represents the size of the max square sub matrix with all 1s,
        whose lowest-rightest element is example_matrix[i, j].
    Layer of element:
        Layer_0: [0,0]
        Layer_1: [0,1][1,0]
        Layer_2: [0,2][1,1][2,0]
        Layer_3: [0,3][1,2][2,1][3,0]
        Layer_4: [0,4][1,3][2,2][3,1][4,0]
        Layer_5: [0,5][1,4][2,3][3,2][4,1][5,0]
        Layer_6: [5,1][4,2][3,3][2,4][1,5]
        Layer_7: [5,2][4,3][3,4][2,5]
        Layer_8: [5,3][4,4][3,5]
        Layer_9: [5,4][4,5]
        Layer_10:[5,5]

Algorithm:
    1. SubMatrixSize[i, j] = example_matrix[i, j] when i === 0 or j === 0;
    2. SubMatrixSize[i, j] = example_matrix[i, j] === 0 ? 0 :
       Min(SubMatrixSize[i - 1, j], SubMatrixSize[i - 1, j - 1], SubMatrixSize[i, j - 1]) + 1;

By the above algorithm, the lower-righter layer (with larger layer #)
can be evaluated from the results of the upper-lefter layer (with smaller layer #).
There are no dependencies between evaluation of the elements in the same layer,
so evaluation of the elements in the same layer could be parallel.
*/
async function max_square_sub_matrix_with_all_1s(squareMatrix) {
    var n = squareMatrix.length;
    var layer = 0;

    // All the layers are evaluated in sequence from upp-left to low-right.
    while (layer < 2 * n - 1) {
        await zone.execute('', 'max_square_sub_matrix_with_all_1s_at_layer', [ squareMatrix, layer ]);
        layer++;
    }
}

function max_square_sub_matrix_with_all_1s_at_layer(squareMatrix, layer) {
    var n = squareMatrix.length;
    var promises = [];
    var promiseIndex = 0;

    // Evaluate the elements in the current layer in parallel.
    if (layer < n) {
        for (var i = 0; i <= layer; i++) {
            promises[promiseIndex++] =
                zone.execute('', 'max_square_sub_matrix_with_all_1s_ended_at', [ squareMatrix, i, layer - i ]);
        }
    } else {
        for (var j = layer - n + 1; j < n; j++) {
            promises[promiseIndex++] =
                zone.execute('', 'max_square_sub_matrix_with_all_1s_ended_at', [ squareMatrix, layer - j, j ]);
        }
    }

    return Promise.all(promises).then(result => {});
}

// Evaluate the size of max square sub matrix with all 1s
// whose lowest-rightest element is the squareMatrix[i, j].
function max_square_sub_matrix_with_all_1s_ended_at(squareMatrix, i, j) {
    if (i === 0 || j === 0) {
        store.set(get_key_of_store(i, j), squareMatrix[i][j]);
    } else {
        store.set(get_key_of_store(i, j),
                  squareMatrix[i][j] === 0 ? 0 : Math.min(store.get(get_key_of_store(i - 1, j)),
                                                          store.get(get_key_of_store(i, j - 1)),
                                                          store.get(get_key_of_store(i - 1, j - 1))) +
                                                     1);
    }
}

function get_key_of_store(i, j) {
    return i + '-' + j;
}

function print_result(squareMatrix, ms) {
    var n = squareMatrix.length;
    var maxSize = 0, maxI = 0, maxJ = 0;
    for (var i = 0; i < n; i++) {
        for (var j = 0; j < n; j++) {
            var maxIJ = subMatrixSizeStore.get(get_key_of_store(i, j));
            if (maxIJ > maxSize) {
                maxI = i;
                maxJ = j;
                maxSize = maxIJ;
            }
        }
    }

    console.log();
    console.log(squareMatrix);

    console.log();
    console.log('Max square sub matrix with all 1s');
    console.log('-------------------------------------');
    console.log('    I ends at\t\t:', maxI);
    console.log('    J ends at\t\t:', maxJ);
    console.log('    matrix size\t\t:', maxSize);
    console.log('    # of workers\t:', NUMBER_OF_WORKERS);
    console.log('    Latency in MS\t:', ms);
    console.log('-------------------------------------');
}

// Run program.
run();
