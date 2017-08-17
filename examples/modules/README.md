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
 ## How to build and test ?
 For the module examples, node-gyp or cmake-js build solution is provided for your reference.

Please make sure [node-gyp](https://github.com/nodejs/node-gyp#installation) or [cmake-js](https://github.com/cmake-js/cmake-js#installation) has been set up correctly at your client, then you could follow the below steps to build and test the module examples.
 ```
 1. go to the directory of a module example
 2. run "npm install" to build the module
 3. run "npm test" to build and run the module test, as well as its unittest if any
 ```
