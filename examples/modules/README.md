## Napa module source tree

The source codes can be organized as the structure below.
```
+-- napa-module-example
    +-- inc
    |   +-- example.h
    +-- src
    |   +-- example.cpp
    |   +-- addon.h
    |   +-- addon.cpp
    +-- lib
    |   +-- example.ts
    |   +-- tsconfig.json
    +-- test
    |   +-- test.ts
    |   +-- tsconfig.json
    +-- binding.gyp (when build configuration is given in node-gyp)
    +-- CMakeLists.txt (when build configuration is given in cmake-js)
    +-- package.json
    +-- README.md
```
 ## How to build and run ?
 Cross-platform build system is recommended to build napa modules. For the module examples, [node-gyp](https://github.com/nodejs/node-gyp#installation) or [cmake-js](https://github.com/cmake-js/cmake-js#installation) build solution is provided for your reference.

Please follow the above links to set up node-gyp or cmake-js at your client, and run the below steps to build and run the module examples.
 ```
 1. go the one of the module examples directory
 2. run "npm install" to build the example module
 3. run "npm test" to build and run unittest and module-test
 ```
