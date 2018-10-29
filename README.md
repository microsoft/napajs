[![Build Status for Linux/MacOS](https://travis-ci.org/Microsoft/napajs.svg?branch=master)](https://travis-ci.org/Microsoft/napajs)
[![Build Status for Windows](https://ci.appveyor.com/api/projects/status/github/Microsoft/napajs?branch=master&svg=true)](https://ci.appveyor.com/project/napajs/napajs)
[![npm version](https://badge.fury.io/js/napajs.svg)](https://www.npmjs.com/package/napajs)
[![Downloads](https://img.shields.io/npm/dm/napajs.svg)](https://www.npmjs.com/package/napajs)

# Napa.js
Napa.js is a multi-threaded JavaScript runtime built on [V8](https://github.com/v8/v8), which was originally designed to develop highly iterative services with non-compromised performance in Bing. As it evolves, we find it useful to complement [Node.js](https://nodejs.org) in CPU-bound tasks, with the capability of executing JavaScript in multiple V8 isolates and communicating between them. Napa.js is exposed as a Node.js module, while it can also be embedded in a host process without Node.js dependency.

## Installation
Install the latest stable version:
```
npm install napajs
```
Other options can be found in [Build Napa.js](https://github.com/Microsoft/napajs/wiki/build-napa.js).

## Quick Start
```js
const napa = require('napajs');
const zone1 = napa.zone.create('zone1', { workers: 4 });

// Broadcast code to all 4 workers in 'zone1'.
zone1.broadcast('console.log("hello world");');

// Execute an anonymous function in any worker thread in 'zone1'.
zone1.execute(
    (text) => text, 
    ['hello napa'])
    .then((result) => {
        console.log(result.value);
    });
```
More examples:
* [Estimate PI in parallel](./examples/tutorial/estimate-pi-in-parallel)
* [Max sub-matrix of 1s with layered parallelism](./examples/tutorial/max-square-sub-matrix)
* [Parallel Quick Sort](./examples/tutorial/parallel-quick-sort)
* [Recursive Fibonacci with multiple JavaScript threads](./examples/tutorial/recursive-fibonacci)
* [Synchronized loading](./examples/tutorial/synchronized-loading)

## Features
- Multi-threaded JavaScript runtime.
- Node.js compatible module architecture with NPM support.
- API for object transportation, object sharing and synchronization across JavaScript threads.
- API for pluggable logging, metric and memory allocator.
- Distributed as a Node.js module, as well as supporting embed scenarios.

## Documentation
- [Napa.js Home](https://github.com/Microsoft/napajs/wiki)
- [API Reference](./docs/api/index.md)
- [FAQ](https://github.com/Microsoft/napajs/wiki/FAQ)

# Contribute
You can contribute to Napa.js in following ways:

* [Report issues](https://github.com/Microsoft/napajs/issues) and help us verify fixes as they are checked in.
* Review the [source code changes](https://github.com/Microsoft/napajs/pulls).
* Contribute to core module compatibility with Node.
* Contribute bug fixes.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).<br> For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact opencode@microsoft.com with any additional questions or comments.

# License
Copyright (c) Microsoft Corporation. All rights reserved.

Licensed under the [MIT](https://github.com/Microsoft/napajs/blob/master/LICENSE.txt) License.
