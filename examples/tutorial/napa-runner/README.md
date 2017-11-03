# Napa runner
Napa runner is a tool to run JavaScript with napa capability. It is implemented in C++ using napa API.

## How to build
1. Go to napajs root directory, run "node build.js embed" to build napa library for embeded mode. 
2. Go to directory of "examples/tutorial/napa-runner", and run "cmake-js build" to build napa runner

**NOTE**: The 1st step may take about 30 mins, because it will build V8 libray from node-v6.10.3.

## How to use
1. Run "npm install" to intall required npm modules
2. Run "./bin/NapaRunner emstimate-PI.js"

**NOTE**: Build solution of NapaRunner is provided only for linux system so far. Windows / Mac OS support will come in near future.
