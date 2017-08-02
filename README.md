# Napa.js
Napa.js is a multi-threaded JavaScript runtime built on [V8](https://github.com/v8/v8), which was originally designed to develop highly iterative services with non-compromised performance in Bing. As it evolves, we find it useful to complement [Node.js](https://nodejs.org) in CPU-bound tasks, with the capability of executing JavaScript in multiple V8 isolates and communicating between them. Napa.js is exposed as a Node.js module, while it can also be embedded in a host process without Node.js dependency.

## Supported OS and Compilers
Napa.js requires C++ compiler that supports [C++14](https://en.wikipedia.org/wiki/C%2B%2B14), currently we have tested following OS/compiler combinations: 
* Windows: 7+ with Visual Studio 2015 or 2017
* Linux: Ubantu 14.04 LTS, 16.04 TLS, with gcc 5.4+ 
* OSX: 10.11 (Yosemite), Apple LLVM 7.0.2 (clang-700.1.18)

## Installation
### Prerequisites
* Install C++ compilers that support VC14: 
    * Windows: [VC 2015 Build Tools](http://landinghub.visualstudio.com/visual-cpp-build-tools) or [VS 2017 Build Tools](https://www.visualstudio.com/downloads/#build-tools-for-visual-studio-2017)
    * Linux: [via Apt-get](https://askubuntu.com/questions/618474/how-to-install-the-latest-gcurrently-5-1-in-ubuntucurrently-14-04)
    * OSX: `xcode-select --install`
* Install CMake: [Download or build](https://cmake.org/install/), [Apt-get (Linux/Ubuntu)](https://askubuntu.com/questions/355565/how-to-install-latest-cmake-version-in-linux-ubuntu-from-command-line)
* Install cmake-js: `npm install -g cmake-js`

### Install Napa.js
```
npm install napajs
```
\* *You can also [build napajs from source](https://github.com/Microsoft/napajs/wiki/build-'napajs'-from-source).*

## Quick Start
```js
var napa = require('napajs');
var zone1 = napa.zone.create('zone1', { workers: 4} );

// Broadcast code to all 4 workers in 'zone1'.
zone1.broadcast('console.log("hello world");');

// Execute an anonymous function in any worker thread in 'zone1'.
zone1.execute(
    (text) => {
        return text;
    }, 
    ['hello napa'])
    .then((result) => {
        console.log(result.value);
    });
```
More examples:
* [Estimate PI in parallel](./examples/tutorial/estimate-pi-in-parallel)
* [Recursive Fibonacci with multiple JavaScript threads](./examples/tutorial/recursive-fibonacci)
* [Max sub-matrix of 1s with layered parallelism](./examples/tutorial/max-square-sub-matrix)

## Features
- Multi-threaded JavaScript runtime
- Node.js compatible module architecture with NPM support
- API for object transportation, object sharing and synchronization across JavaScript threads
- API for pluggable logging, metric and memory allocator
- Distributed as a Node.js module, as well as supporting embed scenarios

## Documentation
- [Napa.js Home](https://github.com/Microsoft/napajs/wiki)
- [API Reference](./docs/api/index.md)

# Contribute
You can contribute to Napa.js in following ways:

* [Report issues](https://github.com/Microsoft/napajs/issues) and help us verify fixes as they are checked in.
* Review the [source code changes](https://github.com/Microsoft/napajs/pulls).
* Contribute bug fixes.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact opencode@microsoft.com with any additional questions or comments.

# Contacts
* [Mailing list](https://groups.google.com/forum/#!forum/napajs)
