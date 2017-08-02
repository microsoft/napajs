# Napa.js Modules

## Table of Contents
- [Introduction](#intro)
- [Developing modules](#develop-modules)
  - [Module: JavaScript vs C++](#js-vs-cpp)
- [Quick reference](#quick-ref)
  - [JavaScript module](#ref-js-module)
  - [C++ module](#ref-cpp-module)
- [API](#api)
  - [JavaScript API](#js-api)
  - [C++ API](#cpp-api)
    - [Exporting JavaScript class from C++ modules](#export-class)
    - [V8 helpers](#v8helpers)
    - [Using STL with custom allocators](#stl-with-allocator)
- [Special topics](#topics)
  - [Topic #1: Make objects shareable across multiple JavaScript threads](#topic-shareable-objects)
  - [Topic #2: Asynchronous functions](#topic-async-functions)
  - [Topic #3: Memory management in C++ modules](#topic-memory-management)

## <a name="intro"></a> Introduction
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


## <a name="develop-modules"></a> Developing modules
### <a name="js-vs-cpp"></a> Module: JavaScript vs. C++
A quick glance at NPM will reveal that most modules are pure JavaScript. These are only a few reasons that you may want to create a C++ module.
- You want to expose JavaScript API for existing C/C++ functionalities.
- Code includes considerably amount of computation that is performance critical.
- Objects need to be shared across multiple JavaScript threads, marshalling/unmarshalling cost on these objects is not trivial (big payload size, complex structure, etc.), but it's reasonable cheap to expose JavaScript APIs from underlying native objects.
- In embed mode, you want to communicate with host process with native objects.

[This post](https://docs.npmjs.com/getting-started/creating-node-modules) gives a good introduction on creating a JavaScript module. For creating a Napa.JS C++ module, please refer to the [API](#api) section or checkout examples in the [quick reference](#quick-reference) section.

## <a name="quick-ref"></a> Quick reference

### <a name="ref-js-module"></a> JavaScript module

| Description                                                  | Transportable | Example code |
| ------------------------------------------------------------ | ------------- | ------------ |
| Standard JavaScript module                                   |               | [Blog post](https://www.hacksparrow.com/how-to-write-node-js-modules.html)           |
| Share JavaScript object across isolates                      |      X        |              |

### <a name="ref-cpp-module"></a> C++ module

| Description                                                  | ObjectWrap | Transportable | Async function | Example code |
| ------------------------------------------------------------ | ---------- | ------------- | -------------- | ------------ |
| Export JavaScript function only                              |            |               |                |  hello-world [[.md](../../examples/modules/hello-world/README.md) [.cpp](../../examples/modules/hello-world/node/addon.cpp) [test](../../examples/modules/hello-world/test/test.ts)]                           |
| Export JavaScript object (ObjectWrap)                        |      X     |               |                |  plus-number [[.md](../../examples/modules/plus-number/README.md) [.cpp](../../examples/modules/plus-number/node/addon.cpp) [test](../../examples/modules/plus-number/test/module-test/test.ts)]            |
| Share C++ object across isolates                             |      X     |      X        |                |  allocator-wrap [[.h](../../src/module/core-modules/napa/allocator-wrap.h) [.cpp](../../src/module/core-modules/napa/allocator-wrap.cpp)]            |
| Export asynchronous JavaScript function                      |      X     |               |      X         |  async-number [[.md](../../examples/modules/async-number/README.md) [.cpp](../../examples/modules/async-number/node/addon.cpp) [test](../../examples/modules/async-number/test/test.ts)]            |

## <a name="api"></a> API
### <a name="js-api"></a> JavaScript
See [API reference](./index.md).

### <a name="cpp-api"></a> C++
#### <a name="export-class"></a> Exporting JavaScript classes from C++ modules
TBD
#### <a name="v8helpers"></a> V8 helpers
TBD
#### <a name="stl-with-allocator"></a> Using STL with custom allocators
TBD

## <a name="topics"></a> Special topics
### <a name="topic-shareable-objects"></a> Topic #1: Make objects shareable across multiple JavaScript threads
TBD

### <a name="topic-async-functions"></a> Topic #2: Asynchronous functions
TBD

### <a name="topic-memory-management"></a> Topic #3: Memory management in C++ modules
TBD
