# Parallel Quick Sort
This example implements a parallel version of quicksort by napa.js, and compares its performance to that of a serial version. It demonstrates
1. How does napa.js transport a TypedArray (JavaScript built-in objects) among napa worker transparently. The TypedArray is created from a SharedArrayBuffer.
2. How does napa.js accelerate a computation heavy task by parallelization.

## How to run
1. Go to directory of "examples/tutorial/parallel-quick-sort"
2. Run "npm install" to install napajs
3. Run "node parallel-quick-sort.js"

**Note**: This example involves TypedArray and SharedArrayBuffer. It requires Node v9.0.0 or newer.

## Program output
The table below shows results of the serial quicksort and the parallel one.
```
quickSort:              It took ( 1006 ) MS to sort an array with  4194304  elements.
parallelQuickSort:      It took ( 388 ) MS to sort an array with  4194304  elements.
```
Both of them were executed on a 4 millions length Float64Array. The parallel implementation by napa.js brought 60+% performance gain under my test environment below.

| Name              | Value                                                                                 |
|-------------------|---------------------------------------------------------------------------------------|
|**Processor**      |Intel(R) Xeon(R) CPU E5-2620 0 @ 2.00GHz, 2000 Mhz, 6 Core(s), 12 Logical Processor(s) |
|**System Type**    |x64-based PC                                                                           |
|**Physical Memory**|32.0 GB                                                                                |
|**OS version**     |Microsoft Windows Server 2016 Datacenter                                               |
