# Napa runner
Napa runner is an example to embed Napa.JS into a C++ program. It simply runs JavaScript with all Napa capability without Node dependency from command line.

## How to build
1. Go to napajs root directory, run "node build.js embed" to build napa library for embeded mode. 

**NOTE**: This step may take about 30 mins, because it will build V8 libray from Node. We are using node v6.10.3, a stable version can build as a share library. It is specified in [embedded.js](https://github.com/Microsoft/napajs/blob/master/scripts/embedded.js) and [napa-runner CMakeLists.txt](https://github.com/Microsoft/napajs/blob/master/examples/tutorial/napa-runner/CMakeLists.txt). Please update both of them if you want to use a different version of Node/V8.

2. Go to directory of "examples/tutorial/napa-runner", and run "cmake-js build" to build napa runner

**NOTE**: Build solution of napa-runner is provided only for linux system so far. Windows / Mac OS support will come in near future.

## How to use
1. Run "npm install" to intall required npm modules
2. Run "./bin/napa-runner emstimate-PI.js"

