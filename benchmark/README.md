# Benchmark

## Summary
- JavaScript execution in napajs is on par with node, using the same version of V8, which is expected.
- `zone.execute` scales linearly on number of workers, which is expected.
- The overhead of calling `zone.execute` from node is around 0.1ms after warm-up. The cost of using anonymous function is neglectable.
- `transport.marshall` cost on small plain JavaScript values is about 3x of JSON.stringify.
- The overhead of `store.set` and `store.get` is around 0.06ms plus transport overhead on the objects.

We got this report on environment below:

| Name              | Value                                                                                 |
|-------------------|---------------------------------------------------------------------------------------|
|**Processor**      |Intel(R) Xeon(R) CPU L5640 @ 2.27GHz, 8 virtual procesors                              |
|**System Type**    |x64-based PC                                                                           |
|**Physical Memory**|16.0 GB                                                                                |
|**OS version**     |Microsoft Windows Server 2012 R2                                                       |


## Napa vs. Node on JavaScript execution 
Please refer to [node-napa-perf-comparison.ts](node-napa-perf-comparison.ts).

| node time | napa time |
| --------- | --------- |
| 3026.76   | 3025.81   |

## Linear scalability
`zone.execute` scales linearly on number of workers. We performed 1M CRC32 calls on a 1024-length string on each worker, here are the numbers. We still need to understand why the time of more workers running parallel would beat less workers.

|         | node        | napa - 1 worker | napa - 2 workers | napa - 4 workers | napa - 8 workers |
| ------- | ----------- | --------------- | ---------------- | ---------------- | ---------------- |
| time    | 8,649521600 | 6146.98         | 4912.57          | 4563.48          | 6168.41          |
| cpu%    | ~15%        | ~15%            | ~27%             | ~55%             | ~99%             |

Please refer to [execute-scalability.ts](./execute-scalability.ts) for test details.

## Execute overhead
The overhead of `zone.execute` includes
1. Marshalling cost of arguments in caller thread.
2. Queuing time before a worker can execute.
3. Unmarshalling cost of arguments in target worker.
4. Marshalling cost of return value from target worker.
5. Queuing time before caller callback is notified. 
6. Unmarshalling cost of return value in caller thread.

In this section we will examine #2 and #5. So we use empty function with no arguments and no return value.

Transport overhead (#1, #3, #4, #6) varies by size and complexity of payload, will be benchmarked separately in [Transport Overhead](#transport-overhead) section.

Please refer to [execute-overhead.ts](./execute-overhead.ts) for test details.

### Overhead after warm-up
Average overhead is around 0.06ms to 0.12ms for `zone.execute`.

| repeat   | zone.execute (ms) |
|----------|-------------------|
| 200      | 24.932            |
| 5000     | 456.893           |
| 10000    | 810.687           |
| 50000    | 3387.361          |

*10000 times of zone.execute on anonymous function is 807.241ms. The gap is within range of bench noise.

### Overhead during warm-up:

| Sequence of call | Time (ms) |
|------------------|-----------|
| 1                |  6.040    |
| 2                |  4.065    |
| 3                |  5.250    |
| 4                |  4.652    |
| 5                |  1.572    |
| 6                |  1.366    |
| 7                |  1.403    |
| 8                |  1.213    |
| 9                |  0.450    |
| 10               |  0.324    |
| 11               |  0.193    |
| 12               |  0.238    |
| 13               |  0.191    |
| 14               |  0.230    |
| 15               |  0.203    |
| 16               |  0.188    |
| 17               |  0.188    |
| 18               |  0.181    |
| 19               |  0.185    |
| 20               |  0.182    |


## Transport overhead

The overhead of `transport.marshall` includes
1. overhead of needing replacer callback during JSON.stringify. (even empty callback will slowdown JSON.stringfiy significantly)
2. traverse every value during JSON.stringify, to check value type and get `cid` to put into payload.
    - a. If value doesn't need special care.
    - b. If value is a transportable object that needs special care.

2.b is related to individual transportable classes, which may vary per individual class. Thus we examine #1 and #2.a in this test.

The overhead of `transport.unmarshall` includes
1. overhead of needing reviver callback during JSON.parse.
2. traverse every value during JSON.parse, to check if object has `_cid` property.
    - a. If value doesn't have property `_cid`.
    - b. Otherwise, find constructor and call the [`Transportable.marshall`](../docs/api/transport.md#transportable-marshall).

We also evaluate only #1, #2.a in this test.

Please refer to [transport-overhead.ts](./transport-overhead.ts) for test details.

\*All operations are repeated for 1000 times.

| payload type                       | size  | JSON.stringify (ms) | transport.marshall (ms) | JSON.parse (ms) | transport.unmarshall (ms) |
| ---------------------------------- | ----- | ------------------- | ----------------------- | --------------- | ------------------------- |
| 1 level - 10 integers              | 91    | 4.90                | 18.05 (3.68x)           | 3.50            | 17.98 (5.14x)             |
| 1 level - 100 integers             | 1081  | 65.45               | 92.78 (1.42x)           | 20.45           | 122.25 (5.98x)            |
| 10 level - 2 integers              | 18415 | 654.40              | 2453.37 (3.75x)         | 995.02          | 2675.72 (2.69x)           |
| 2 level - 10 integers              | 991   | 19.74               | 66.82 (3.39x)           | 27.85           | 138.45 (4.97x)            |
| 3 level - 5 integers               | 1396  | 33.66               | 146.33 (4.35x)          | 51.54           | 189.07 (3.67x)            |
| 1 level - 10 strings - length 10   | 201   | 3.81                | 10.17 (2.67x)           | 9.46            | 20.81 (2.20x)             |
| 1 level - 100 strings - length 10  | 2191  | 76.53               | 115.74 (1.51x)          | 77.71           | 181.24 (2.33x)            |
| 2 level - 10 strings - length 10   | 2091  | 30.15               | 97.65 (3.24x)           | 95.51           | 213.20 (2.23x)            |
| 3 level - 5 strings - length 10    | 2646  | 41.95               | 155.42 (3.71x)          | 123.82          | 227.90 (1.84x)            |
| 1 level - 10 strings - length 100  | 1101  | 7.74                | 12.19 (1.57x)           | 17.34           | 29.83 (1.72x)             |
| 1 level - 100 strings - length 100 | 11191 | 66.17               | 112.83 (1.71x)          | 197.67          | 282.63 (1.43x)            |
| 2 level - 10 strings - length 100  | 11091 | 68.46               | 149.99 (2.19x)          | 202.85          | 298.19 (1.47x)            |
| 3 level - 5 integers               | 13896 | 89.46               | 208.21 (2.33x)          | 265.25          | 418.42 (1.58x)            |
| 1 level - 10 booleans              | 126   | 2.84                | 8.14 (2.87x)            | 3.06            | 14.20 (4.65x)             |
| 1 level - 100 booleans             | 1341  | 20.28               | 59.36 (2.93x)           | 21.59           | 121.15 (5.61x)            |
| 2 level - 10 booleans              | 1341  | 23.92               | 89.62 (3.75x)           | 31.84           | 137.92 (4.33x)            |
| 3 level - 5 booleans               | 1821  | 36.15               | 138.24 (3.82x)          | 55.71           | 195.50 (3.51x)            |

## Store access overhead

The overhead of `store.set` includes
1. Overhead of calling `transport.marshall` on value.
2. Overhead of put marshalled data and transport context into C++ map (with exclusive_lock).

The overhead of `store.get` includes
1. Overhead of getting marshalled data and transport context from C++ map (with shared_lock).
2. Overhead of calling `transport.unmarshall` on marshalled data.

For `store.set`, numbers below indicates the cost beyond marshall is around 0.07~0.4ms varies per payload size. (10B to 18KB). `store.get` takes a bit more: 0.06~0.9ms with the same payload size variance. If the value in store is not updated frequently, it's always good to cache it in JavaScript world.

Please refer to [store-overhead.ts](./store-overhead.ts) for test details.

\*All operations are repeated for 1000 times.

| payload type                       | size  | transport.marshall (ms) | store.save (ms) | transport.unmarshall (ms) | store.get (ms) |
| ---------------------------------- | ----- | ----------------------- | --------------- | ------------------------- | -------------- |
| 1 level - 1 integers               | 10    | 2.54                    | 73.85           | 3.98                      | 65.57          |
| 1 level - 10 integers              | 91    | 8.27                    | 98.55           | 17.23                     | 90.89          |
| 1 level - 100 integers             | 1081  | 97.10                   | 185.31          | 144.75                    | 274.39         |
| 10 level - 2 integers              | 18415 | 2525.18                 | 2973.17         | 3093.06                   | 3927.80        |
| 2 level - 10 integers              | 991   | 71.22                   | 174.01          | 154.76                    | 276.04         |
| 3 level - 5 integers               | 1396  | 127.06                  | 219.73          | 182.27                    | 337.59         |
| 1 level - 10 strings - length 10   | 201   | 14.43                   | 79.68           | 31.28                     | 84.71          |
| 1 level - 100 strings - length 10  | 2191  | 104.40                  | 212.44          | 173.32                    | 239.09         |
| 2 level - 10 strings - length 10   | 2091  | 79.54                   | 188.72          | 189.29                    | 252.83         |
| 3 level - 5 strings - length 10    | 2646  | 155.14                  | 257.78          | 276.22                    | 342.95         |
| 1 level - 10 strings - length 100  | 1101  | 15.22                   | 89.84           | 30.87                     | 88.18          |
| 1 level - 100 strings - length 100 | 11191 | 119.89                  | 284.05          | 287.17                    | 403.77         |
| 2 level - 10 strings - length 100  | 11091 | 137.10                  | 299.32          | 244.13                    | 297.12         |
| 3 level - 5 integers               | 13896 | 183.84                  | 310.89          | 285.80                    | 363.50         |
| 1 level - 10 booleans              | 126   | 5.74                    | 49.89           | 22.69                     | 97.27          |
| 1 level - 100 booleans             | 1341  | 57.41                   | 157.80          | 106.30                    | 218.05         |
| 2 level - 10 booleans              | 1341  | 76.93                   | 150.25          | 104.02                    | 185.82         |
| 3 level - 5 booleans               | 1821  | 102.47                  | 171.44          | 150.42                    | 207.27         |
