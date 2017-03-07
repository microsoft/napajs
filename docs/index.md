# Napa.JS: A multi-thread JavaScript runtime

## Introduction
If you have ever used Node.JS, building a website or a mobile application, or using it just as a tool, you must find it fascinating on how fast we can build new things from scratch. The eco-system of NPM lifts us up to innovate on top of others' work, and meanwhile share code that others can depend on. Furthermore, Javascript as a dynamic language can boost productivity in another 2x to 10x depends on the complexity of system.

Node.JS is widely used as a platform. Its [event loop](https://nodesource.com/blog/understanding-the-nodejs-event-loop/) works well with Javascript language just like beans and carrots. 
However, Node.JS' event loop is blessed to a single thread (a V8 isolate), so if your code runs slow, it will block the event loop and other events will be delayed. This is especially an issue for server-side program on multiple cores, which is very common today. Putting computation heavy tasks into C++ add-on and use async calls will solve this problem in some degree, but developer loses agility and productivity writing code in JavaScript, not mentioning sometimes it's not straightforward to partition lengthy logics into bi-parties of JavaScript and C++. 

[Node.JS Cluster](https://nodejs.org/api/cluster.html) is a suggested way to scale computation on multiple-cores, using multiple Node.JS processes, with APIs that can communicate between each other. But when a process is consuming a large amount of memory, the multi-process solution doesn't work that well, as it's very difficult to share memory with structures across processes, and inter-process communication are very expensive. This scenario is very common today in services that load big machine learned models.

Napa.JS is a solution for serving computation-heavy code in JavaScript. It creates *Container* that manages a pool of threads (V8 isolates) that run JavaScript. These isolates are not Node.JS isolate, but isolates with best-effort compatibility with Node.JS APIs, including support of modules, so we can run Node.JS modules inside Napa.JS runtime as many as possible. Napa.JS also provides a set of APIs to share C++ objects and collaborate between isolates. The goal of Napa.JS is not to replace Node.JS, but to extend Node.JS spectrum on computation-heavy tasks. Napa.JS is exposed as a Node.JS module ('napajs'), that provides API to enter Napa world from Node.JS world. Napa.JS can also be embeded in other languages like C++, C#, etc.

## Quick Start
### Install Napa.JS
If you are using Napa.JS under Node.JS, you can install Napa.JS from NPM:
```
npm install napajs
```

If you want to embed Napa.JS into a C++ or C# process, you can install Napa.JS as a Nuget package:
```
TODO:
```

### Use Napa.JS
Napa.JS exposes its functionality via *Container* APIs. A *Container* is a JavaScript runtime that consists of a pool of V8 isolates, which bind to the same number of threads. All threads load the same JavaScript file via *container.loadFile* or *container.loadFileSync* method, which creates objects and functions that can be called later by *container.run* or *container.runSync*. 

Multiple instances of *Container* can exist in the same process. Different instances can apply different runtime policies, such as V8 heap size, etc.

#### Example in Node.JS

main.js
```js
var napa = require('napajs');

// Create a container and load entry JavaScript file.
var container = napa.createContainer();
container.loadFileSync('container-main.js');

// Run a heavy task without blocking Node.JS event loop.
container.run('sumTill', [1000000], (errorCode, resultString, errorMessage) => {
    console.log(resultString);
});

```

container-main.js
```js

function sumTill(num) {
    var sum = 0;
    for (var i = 1; i <= num; ++i) {
        sum += i
    }
    return sum;
}
```

See also: [[Embedder's guide]](./guide/embedders.md)


## Key Features
- [Container: Multi-threaded JavaScript runtime](./core/container.md)
- [Benchmark: High performance, low latency](./benchmarks/index.md)
- [JavaScript and C++ modules with NPM support](./core/module-system.md) 
- [Incremental Node.JS compatibility](./core/node-compatibility.md)

## Index of Napa.JS Modules
- [Napa.JS built-ins and core modules](./core/built-ins-and-core-modules.md)
- [Napa.JS modules@napajs](./modules/napa-module-index.md)
- [Node.JS modules that works for Napa](./modules/node-module-index.md)

## Developer Guide
- [Guide for embedders](./guide/embedders.md)
- [Coding in JavaScript/TypeScript](./guide/coding.md)
- [Creating C++ modules](./guide/building-cpp-addons.md)
- [Debugging Napa.JS modules](./guide/debugging.md)
