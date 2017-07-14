exports.done = false;
var cycle_b = require('./cycle-b.js');
if (cycle_b.done == true) {
    exports.done = true;
}
