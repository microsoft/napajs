# Recursive Fibonacci
This example implements an algorithm to calculate [Fibonacci numbers](https://en.wikipedia.org/wiki/Fibonacci_number) in a distributed recursive way. 

Recursion is supported on a single thread by JavaScript language, how if we want to involve multiple JavaScript threads to collaborate on a task? Since recursion will block caller until callee returns, dispatching recursive tasks to other threads will soon block all threads, which leads to deadlock. This example demonstrates recursive dispatching using [Promise](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Promise), that is, current thread will continue to serve other tasks while its sub-tasks are pending, and resume work once sub-tasks complete. 

Please note that this example is to demostrate the programming paradigm, while itself is *NOT* performance efficient, since each worker does too little CPU operation (simply '+') and major overhead is on communication. 

## How to run
1. Go to directory of "examples/tutorial/recursive-fibonacci"
2. Run "npm install" to install napajs
3. Run "node recursive-fibonacci.js"

## Program output
Table below shows statistics of calculating Nth Fibonacci number by a napa zone with 4 workers.
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
We got results under environment:

| Name              | Value                                                                                 |
|-------------------|---------------------------------------------------------------------------------------|
|**Processor**      |Intel(R) Xeon(R) CPU E5-2620 0 @ 2.00GHz, 2000 Mhz, 6 Core(s), 12 Logical Processor(s) |
|**System Type**    |x64-based PC                                                                           |
|**Physical Memory**|32.0 GB                                                                                |
|**OS version**     |Microsoft Windows Server 2016 Datacenter                                               |
