# Estimate PI in parallel
This example is designed to demonstrate concurrent executions of tasks across multiple workers of napa zone.

With Monte Carlo method to evaluate PI, multiple batches of points were evaluated by a napa zone asynchronously. The napa zone was initialized with 4 workers, so different batches could be evaluated concurrently by different workers. After all the batches finished its evalutation, the resutls were aggregated to get the final PI evaluation.

## How to run
1. go to directory of "examples/tutorial/estimate-pi-in-parrallel"
2. run "npm install" to install napajs
3. run "node estimate-pi-in-parrallel.js"

## Program output
The below table shows the result of Monte Carlo with 4,000,000 points evaluted by a napa zone with 4 worker initialized. The execution latency varies for differnt  # of batches the points was splited. The latency of the first 3 executions reduced significantly along with the increasement # of batches (1 -> 2 -> 4), while the last 2 executions shows close latency because only 4 workers are available for 4 / 8 batches.
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
