# Napa.JS
Napa.JS is a multi-threaded JavaScript runtime built on V8 and TypeScript, which was originally designed to develop highly iterative services with non-compromised performance in Bing. As it evolves, we find it useful to complement Node.JS in CPU-bound tasks, with the capability of executing JavaScript in multiple V8 isolates and communicating between them. Napa.JS is exposed as a Node.JS module, while it can also be embedded in a host process without Node.JS dependency.

## Installation
```
npm install napajs
```
## Quick Start
```ts
let napa = require('napajs');
let zone = napa.zone.create('zone1');

// Execute an anonymous function in any worker thread in 'zone1'.
zone.execute(
    (text: string) => {
        return text;
    }, 
    ['hello world'])
    .then((result: napa.zone.Result) => {
        console.log(result.value);
    });
```
## Features
- Multi-threaded JavaScript runtime
- Node.JS compatible module architecture with NPM support
- API for object transportation, object sharing and synchronization across JavaScript threads
- API for pluggable logging, metric and memory allocator
- Distributed as a Node.JS module, as well as supporting embed scenarios

## Benchmarks
- [Napa.JS 1.0.1](benchmark/README.md)
- Histories


## Documentation
- [Napa.JS wiki](https://github.com/Microsoft/napajs/wiki)



## Contribute
Please follow the instructions in [Napa.JS wiki](https://github.com/Microsoft/napajs/wiki/contribute)
