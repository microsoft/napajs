# Napa.js core modules development guideline

## Summary

A Napa.js **core module** is a module that provided by Napa.js and initialized ready for use in a Napa zone.

There are 2 types of core modules:

- **Napa API core modules**: A set of Napa.js specific modules that described in [Napa.js modules](https://github.com/Microsoft/napajs/blob/master/docs/api/index.md#core-modules).

- **Node API core modules**: A set of Napa.js modules that implement a subset of [Node.js modules](https://github.com/Microsoft/napajs/blob/master/docs/api/node-api.md).

All core modules can be imported by using global function `require(<module-name>)` in napa zone. If a module is a built-in module, it is already loaded and can be accessed by its name as a global object.

## Core modules index

Header file [`/src/module/core-modules/core-modules.h`](https://github.com/Microsoft/napajs/blob/master/src/module/core-modules/core-modules.h) lists all native core modules to export.

JSON file [`/lib/core/core-modules.json`](https://github.com/Microsoft/napajs/blob/master/lib/core/core-modules.json) lists all core modules available in Napa zone.

## Napa API core modules

Napa API core modules are implemented in C++ and Typescript.

C++ implementation goes to folder `/src/module/core-modules/napa`. File [`/src/module/core-modules/napa/napa-binding.cpp`](https://github.com/Microsoft/napajs/blob/master/src/module/core-modules/napa/napa-binding.cpp) is the entry of native boundary.

Typescript implementation goes to folder `/lib`. It uses `process.binding()` to access binary core modules and exports whatever defined in API document. File [`/lib/index.ts`](https://github.com/Microsoft/napajs/blob/master/lib/index.ts) is the entry of javascript boundary.

## Node API core modules

Napa API core modules are implemented in C++ and Typescript.

C++ implementation goes to folder `/src/module/core-modules/node`.

Typescript implementation goes to folder `/lib/core`. It uses `process.binding()` to access binary core modules and exports whatever defined in API document.

## Developing guideline

- To add a pure native module, follow the following steps:
   - Create a module class in the correct folder (see above) with a static `Init()` method.
   - Add the class to the list in `/src/module/core-modules/core-modules.h`.
   - Add the module name in `/lib/core/core-modules.json`.

- To add a pure typescript/javascript Node API module, follow the following steps:
   - Add files under folder `/lib/core`.
   - Update file `/lib/core/.gitignore` if any javascript file added under folder `/lib/core`.
   - Add the module name in `/lib/core/core-modules.json`.

- To add a native/js mixed Node API module, follow the steps of both as described above. Use `process.binding()` to access binary core modules in typescript/javascript.

- To add a Napa API module usually means a big change to existing structure. May need a design review.

- Try to always use typescript rather than javascript. Use javascript only when leveraging an existing javascript implementation.

- Node API modules should be tested in Napa zone, and Napa API modules should be tested in both Node and Napa zone.

- Pure native test goes to `/unittest` (Using [Catch](https://github.com/catchorg/Catch2)) and js test goes to `/test` (Using [mocha](https://github.com/mochajs/mocha)).