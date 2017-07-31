# Estimate PI in parallel
This example demonstrates concurrent execution of tasks across multiple workers of napa zone.

With Monte Carlo method to evaluate PI, multiple batches of points were evaluated by a napa zone asynchronously. The napa zone was initialized with 4 workers, so different batches could be evaluated concurrently by different workers. After all the batches finished their evalutation, the resutls were aggregated to get the final PI evaluation.

## How to run
1. go to directory of "examples/tutorial/estimate-pi-in-parrallel"
2. run "npm install" to install napajs
3. run "node estimate-pi-in-parrallel.js"

## Program output
The below table shows the result of Monte Carlo with 4,000,000 points evaluated by a napa zone with 4 workers. The 4,000,000 points is divided into different number of batches for each iteration. For those iterations (1-batch, 2-batch, and 4-batch) whose batch number is less than the worker number, execution latency is proportional to the number of batches. Meanwhile, the 8-batch iteration cannot scale linearly due to insufficient free worker, which is expected.
```
        # of points     # of batches    # of workers    latency in MS   estimated π     deviation
        ---------------------------------------------------------------------------------------
        4000000         1               4               218             3.141958        0.0003653464
        4000000         2               4               110             3.141953        0.0003603464
        4000000         4               4               78              3.139600        0.001992654
        4000000         8               4               62              3.142732        0.001139346
```
The above result was got by running the example in the below environment.

|               |                                                                                       |
|---------------|---------------------------------------------------------------------------------------|
|Processor      |Intel(R) Xeon(R) CPU E5-2620 0 @ 2.00GHz, 2000 Mhz, 6 Core(s), 12 Logical Processor(s) |
|System Type    |x64-based PC                                                                           |
|Physical Memory|32.0 GB                                                                                |
|OS Name        |Microsoft Windows Server 2016 Datacenter                                               |
|               |                                                                                       |
