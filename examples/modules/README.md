## Napa module source tree

The source codes can be organized as the structure below. This is different from npm package structure since npm
 doesn’t contain source tree until we figure it out how to compile Napa module from npm install. This example uses
 msbuild as build system until napa team finds the good one for cross-platform.
```
+-- napa-module-example
    +-- inc
    |   +-- example.h
    +-- src
    |   +-- example.cpp
    |   +-- example.vcxproj
    +-- node
    |   +-- addon.h
    |   +-- addon.cpp
    |   +-- addon.vcxproj
    +-- napa
    |   +-- addon.vcxproj
    +-- script
    |   +-- src
    |   |   +-- example.ts
    |   +-- package.json
    |   +-- script.vcxproj
    |   +-- tsconfig.json
    +-- test
    |   +-- example-test.njsproj
    |   +-- test.ts
    +-- README.md
```

## Napa MPM tree

This is the simple NPM package structure, which has module binaries directly. Once Napa team figure out the build system
 under *npm install*, this will be changed to support cross-platform build and test.
```
+-- napa-module-example
    +-- bin
    |   +-- addon.napa
    |   +-- addon.node
    +-- lib
    |   +-- example.js
    +-- typings
    |   +-- example.d.ts
    +-- package.json
    +-- tsconfig.json
    +-- README.md
```