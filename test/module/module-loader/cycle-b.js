exports.done = false;
var cycle_a = require('./cycle-a.js');
if (cycle_a.done == false) {
    exports.done = true;
}
