# Max square sub matrix
This example implements an algorithm to solve [Max square sub-matrix of all 1s](http://www.geeksforgeeks.org/maximum-size-sub-matrix-with-all-1s-in-a-binary-matrix/) using dynamic programming. It demonstrates parallelism among multiple JavaScript threads via [zone](https://github.com/Microsoft/napajs/wiki/introduction#zone) and cross-thread data sharing via [store](https://github.com/Microsoft/napajs/wiki/introduction#cross-worker-storage).

In this implementation, all units to be evaluated were divided into 2 * size of the square matrix + 1 layers. Units in next layer were evaluated based on results of units from previous layers. Different layers were evaluated in sequence, while units within the same layer were evaluated in parallel, for there is no dependency between them. Results from previous layers were communicated by a store. 

Please note that this example is to demonstrate the programming paradigm, while itself is NOT performance efficient, since each worker does too little CPU operation and major overhead is on communication.

## How to run
1. Go to directory of `examples/tutorial/max-square-sub-matrix`
2. Run `npm install` to install napajs
3. Run `node max-square-sub-matrix.js`

**Note**: This example uses 'async / await', so Node version that supports ES6 is required. (newer than v7.6.0).

## Program output
The output below shows the result of the implementation. By a napa zone with 4 workers, it took 15 ms to find out the max square sub matrix with all 1s for the given 6 * 6 square binary matrix.
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
