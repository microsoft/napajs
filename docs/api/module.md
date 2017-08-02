# Napa.js Modules

## Introduction
Napa.js follows [Node.js' convention](https://nodejs.org/api/modules.html) to support modules, that means:

1) Both JavaScript modules and C++ modules are supported.
2) Module resolution follows the [same algorithm](https://nodejs.org/api/modules.html#modules_all_together), except instead of searching file extension `.node` for addons, Napa.JS searches `.napa`.
3) Supports NPM, with the [same way](https://docs.npmjs.com/getting-started/creating-node-modules) to create and publish modules.
4) API of creating C++ modules (addons) are similar. Napa.JS introduced macros that the same source code can be compiled to produce both Napa.js addon and Node.js addon.

But there are also differences:
1) C++ module that is designed/implemented for Napa.js can run on Node.JS (need different compile flags to produce '.napa' and '.node'). But not vice versal. 
2) Napa.js doesn't support all Node.js API. Node API are supported [incrementally](./node-api.md) on the motivation of adding Node.js built-ins and core modules that are needed for computation heavy tasks. You can access full capabilities of Node exposed via [Node zone](./zone.md#node-zone).
3) Napa.js doesn't provide `uv` functionalities, thus built-ins and core modules have its own implementation. To write async function in addon, methods `DoAsyncWork`/`PostAsyncWork` are introduced to work for both Napa.js and Node.js.
4) Napa.js supports embed mode. C++ modules need separate compilation between Node mode and embed mode.


## Developing Modules
### Module: JavaScript vs. C++
A quick glance at NPM will reveal that most modules are pure JavaScript. These are only a few reasons that you may want to create a C++ module.
- You want to expose JavaScript API for existing C/C++ functionalities.
- Code includes considerably amount of computation that is performance critical.
- Objects need to be shared across multiple JavaScript threads, marshalling/unmarshalling cost on these objects is not trivial (big payload size, complex structure, etc.), but it's reasonable cheap to expose JavaScript APIs from underlying native objects.
- In embed mode, you want to communicate with host process with native objects.

[This post](https://docs.npmjs.com/getting-started/creating-node-modules) gives a good introduction on creating a JavaScript module. For creating a Napa.JS C++ module, please refer to the [API](#api) section or checkout examples in the [quick reference](#quick-reference) section.

## Quick reference

### JavaScript module

| Description                                                  | Transportable | Example code |
| ------------------------------------------------------------ | ------------- | ------------ |
| Standard JavaScript module                                   |               | [Blog post](https://www.hacksparrow.com/how-to-write-node-js-modules.html)           |
| Share JavaScript object across isolates                      |      X        |              |

### C++ module

| Description                                                  | ObjectWrap | Transportable | Async function | Example code |
| ------------------------------------------------------------ | ---------- | ------------- | -------------- | ------------ |
| Export JavaScript function only                              |            |               |                |  hello-world [[.md](../../examples/modules/hello-world/README.md) [.cpp](../../examples/modules/hello-world/node/addon.cpp) [test](../../examples/modules/hello-world/test/test.ts)]                           |
| Export JavaScript object (ObjectWrap)                        |      X     |               |                |  plus-number [[.md](../../examples/modules/plus-number/README.md) [.cpp](../../examples/modules/plus-number/node/addon.cpp) [test](../../examples/modules/plus-number/test/module-test/test.ts)]            |
| Share C++ object across isolates                             |      X     |      X        |                |  allocator-wrap [[.h](../../src/module/core-modules/napa/allocator-wrap.h) [.cpp](../../src/module/core-modules/napa/allocator-wrap.cpp)]            |
| Export asynchronous JavaScript function                      |      X     |               |      X         |  async-number [[.md](../../examples/modules/async-number/README.md) [.cpp](../../examples/modules/async-number/node/addon.cpp) [test](../../examples/modules/async-number/test/test.ts)]            |

## Special topics
### Topic #1: Make objects shareable across multiple JavaScript threads

### Topic #2: Asynchronous functions

### Topic #3: Memory management in C++ modules

## API
### JavaScript
See [API reference](./index.md).

### C++
#### Exporting JavaScript classes from C++ modules
TBD
#### V8 helpers
TBD
#### Using STL with Napa allocators
TBD
