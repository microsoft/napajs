# Napa runner
Napa runner is a tool to run JavaScript with napa capability. It is implemented in C++ using napa API.

## How to build
1. Go to napajs root directory, run "node build.js embed" to build napa library for embeded mode. It will take about 30 mins for the first time, because it will build V8 libray from node-v6.10.3 in this step.
2. Go to directory of "examples/napa-runner"
3. Run "cmake-js build" to build napa runner
## How to use
1. Run "npm install" to intall npm modules required by your testing js
2. Run "./bin/NapaRunner <js-file.js>"

**NOTE**: Build solution of napa runner is provided only for linux system so far. Windows / Mac OS support will come in near future.
