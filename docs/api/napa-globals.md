# Napa.js specific global variables

This file describes Napa.js specific globals, please refer to [this documentation](./node-api.md#globals) for Node.js globals.

## `global.napa`
Shortcut to access `napajs` module in all Napa enabled isolates. This is helpful to avoid extra `require` when using `napajs` module in anonymous function during `broadcast` or `execute`.

Example:
```js
var napa = require('napajs');

var zone = napa.zone.create('zone1');

function test() {
    global.napa.log('hi');
}

zone.execute(test);

```