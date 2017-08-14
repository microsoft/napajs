## Napa module source tree

The source codes can be organized as the structure below. Cross-platform build system is recommended for napa modules. For the module examples, [node-gyp(https://github.com/nodejs/node-gyp#installation) or [cmake-js](https://github.com/cmake-js/cmake-js#installation) build solutions is provided for your reference.
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
    +-- binding.gyp
    +-- CMakeLists.txt
    +-- package.json
    +-- README.md
```
