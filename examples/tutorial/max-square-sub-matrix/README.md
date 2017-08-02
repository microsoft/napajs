# Max square sub matrix
This example demonstrates data communication across workers by using napa store.

In a dynamic programming implementation of this example, all nodes to be evaluated were divided into 2 * size of the square matrix + 1 layers. The nodes in next layer are evaluated based the results of the nodes in the previous nodes. Different layers are evaluated in sequence by napa zone. Results from previous layers were cached and shared across isolates via a store, which are accessed later to evaluate of the nodes in the next layer. There are no dependencies between evaluation of the nodes in the same layer, so evaluation of the nodes in the same layer are performed asynchronously and concurrently by workers of napa zone.

## How to run
1. go to directory of "examples/tutorial/max-square-sub-matrix"
2. run "npm install" to install napajs
3. run "node max-square-sub-matrix.js"

This example uses the js syntax of 'async / await', so we recommend to use the newest version of node.js (newer than v7.6.0) to run it.

## Program output
The below output shows the result of the implementation. By a napa zone with 4 workers, it took 15 ms to find out the max square sub matrix with all 1s for the given 6 * 6 square binary matrix.
```
        [ [ 0, 1, 1, 0, 1, 1 ],
          [ 1, 1, 0, 1, 0, 1 ],
          [ 0, 1, 1, 1, 0, 1 ],
          [ 1, 1, 1, 1, 0, 0 ],
          [ 1, 1, 1, 1, 1, 1 ],
          [ 0, 0, 0, 0, 0, 0 ] ]

        Max square sub matrix with all 1s
        -------------------------------------
            I ends at           : 4
            J ends at           : 3
            matrix size         : 3
            # of workers        : 4
            Latency in MS       : 15
        -------------------------------------
```
We got results under environment:

| Name              | Value                                                                                 |
|-------------------|---------------------------------------------------------------------------------------|
|**Processor**      |Intel(R) Xeon(R) CPU E5-2620 0 @ 2.00GHz, 2000 Mhz, 6 Core(s), 12 Logical Processor(s) |
|**System Type**    |x64-based PC                                                                           |
|**Physical Memory**|32.0 GB                                                                                |
|**OS version**     |Microsoft Windows Server 2016 Datacenter                                               |


The purpose of this example is to demonstrate shared data access across different isolates. It's not performance efficient. Your code should minimize shared data access since it comes with an overload. Please refer to [Benchmark](https://github.com/Microsoft/napajs/blob/master/benchmark/README.md) to see performance details.
