# Recursive fibonacci
This example demonstrates recursive execution of napa tasks.

Fibonacci(n) is evaluated through recursively executing Fibonacci(n - 1) and Fibonacci(n - 2) by a napa zone with 4 workers.

## How to run
1. go to directory of "examples/tutorial/recursive-fibonacci"
2. run "npm install" to install napajs
3. run "node recursive-fibonacci.js"

## Program output
The below table shows the result of evaluations of Fibonacci from 10 to 16 by a napa zone with 4 workers.
```
        Nth     Fibonacci       # of workers    latency in MS
        -----------------------------------------------------------
        10      55              4               46
        11      89              4               47
        12      144             4               47
        13      233             4               47
        14      377             4               94
        15      610             4               140
        16      987             4               266
```
The above result was got by running the example in the below environment.

|               |                                                                                       |
|---------------|---------------------------------------------------------------------------------------|
|Processor      |Intel(R) Xeon(R) CPU E5-2620 0 @ 2.00GHz, 2000 Mhz, 6 Core(s), 12 Logical Processor(s) |
|System Type    |x64-based PC                                                                           |
|Physical Memory|32.0 GB                                                                                |
|OS Name        |Microsoft Windows Server 2016 Datacenter                                               |
|               |                                                                                       |

The purpose of this example is to demonstrate recursive execution of napa tasks. It's not performance efficient. Please refer to [Benchmark](https://github.com/Microsoft/napajs/blob/master/benchmark/README.md) to see performance details.