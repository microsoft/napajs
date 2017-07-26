# Max square sub matrix
This example is designed to demonstrate data communication across workers by leveraging napa store.

In the dynamic programming implementation of this example, all the nodes to be evaluated were splited into 2 * size of the squareMatrix + 1 layers. The nodes in next layer are evaluted based the results of the nodes in the previous nodes. Different layers are evaluted in sequence by napa zone. The results of the nodes in the previous layer were cached in napa store, which are accessed later to evalute of the nodes in the next layer. There are no dependencies between evalution of the nodes in the same layer, so evluation of the nodes in the same layer are performed asynchronously and concurrently by workers of napa zone.

## How to run
1. go to directory of "examples/tutorial/max-square-sub-matrix"
2. run "npm install" to install napajs
3. run "node max-square-sub-matrix.js"

This example uses the js syntax of 'async / await', so we recommend to use the newest version of node.js (newer than v7.6.0) to run it.

## Program output
The below output shows the result of the implementation. By a napa zone with 4 workers initialized, it took 15 ms to find out the max square sub matrix with all 1s for the given 6 * 6 square binary matrix.
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
The above result was got by running the example in the below environment.

|               |                                                                                       |
|---------------|---------------------------------------------------------------------------------------|
|Processor      |Intel(R) Xeon(R) CPU E5-2620 0 @ 2.00GHz, 2000 Mhz, 6 Core(s), 12 Logical Processor(s) |
|System Type    |x64-based PC                                                                           |
|Physical Memory|32.0 GB                                                                                |
|OS Name        |Microsoft Windows Server 2016 Datacenter                                               |
|               |                                                                                       |

BTW, this example is designed to show the usage of napajs by the simplest algorithm. It isn't optimized for performance. Please refer to benchmark for performance of napajs.