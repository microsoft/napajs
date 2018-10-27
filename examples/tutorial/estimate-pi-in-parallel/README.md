# Estimate PI in parallel
This example implements an algorithm to [estimate PI using Monte Carlo method](http://mathfaculty.fullerton.edu/mathews/n2003/montecarlopimod.html). It demonstrates how to fan out sub-tasks into multiple JavaScript threads, execute them in parallel and aggregate output into a final result.

In the implementation, multiple batches of points are evaluated simultaneously in a [napa zone](https://github.com/Microsoft/napajs/wiki/introduction#zone) of 4 workers. Results are aggregated to calculate the final PI after all batches finishes.

## How to run
1. Go to directory of `examples/tutorial/estimate-pi-in-parallel`
2. Run `npm install` to install napajs
3. Run `node estimate-pi-in-parallel.js`

## Program output
The table below shows results of PI calculated under different settings, each setting emulates 4,000,000 points evaluated by a napa zone of 4 workers. 

These 4,000,000 points are divided into multiple batches, each setting differs only in number of batches. For settings (1-batch, 2-batch, and 4-batch) whose batch number is less than the worker number, total latency is proportional to the number of batches, that means we have enough workers to pick up coming batches. On the contrary, the 8-batch setting cannot scale linearly due to insufficient free worker, which is expected.
```
        # of points     # of batches    # of workers    latency in MS   estimated π     deviation
        ---------------------------------------------------------------------------------------
        4000000         1               4               218             3.141958        0.0003653464
        4000000         2               4               110             3.141953        0.0003603464
        4000000         4               4               78              3.139600        0.001992654
        4000000         8               4               62              3.142732        0.001139346
```
We got results under environment:

| Name              | Value                                                                                 |
|-------------------|---------------------------------------------------------------------------------------|
|**Processor**      |Intel(R) Xeon(R) CPU E5-2620 0 @ 2.00GHz, 2000 Mhz, 6 Core(s), 12 Logical Processor(s) |
|**System Type**    |x64-based PC                                                                           |
|**Physical Memory**|32.0 GB                                                                                |
|**OS version**     |Microsoft Windows Server 2016 Datacenter                                               |
