# Estimate PI with embeded napa
Please refer to [estimate PI in Parallel](https://github.com/Microsoft/napajs/tree/master/examples/tutorial/estimate-pi-in-parallel) for the detail of the algorithm.

## How to run
1. Go to napajs root directory, run "node build.js embed" to build napa library for embeded mode. It will take about 30 mins for the first time, because it will build V8 libray from node-v6.10.3 in this step.
2. Go to directory of "examples/tutorial/estimate-PI-with-embeded-napa"
3. Run "cmake-js build" to build this exmaple
4. Run "npm install" to install napajs
5. Run "./bin/estimate-PI-with-embeded-napa"

**NOTE**: Build solution of this exmaple is provided only for linux system so far. Windows / Mac OS support will come in near future.
