# Lazy Loading
This example implements a lazy-loading component. The component should load data when the first time it is used. The lazy loading should be run only once. It is thread-safe in Napa zones with the use of [`napa.sync.Lock`](./../../../docs/api/sync.md#interface-lock).

## How to run
1. Go to directory of "examples/tutorial/lazy-loading"
2. Run "npm install" to install napajs
3. Run "node lazy-loading.js"

## Program output
The output below shows one possible result. The sequence of the output may be different but the data loading will always run once only.
```
[do_something]key=data1, trying to load data...
[do_something]key=data3, trying to load data...
[load_data]loading...
[do_something]key=data2, trying to load data...
[do_something]key=data3, data=<not exist>
[do_something]key=data1, data=foo
[do_something]key=data2, data=bar
[run]All operations are completed.
```
