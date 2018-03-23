console.log(`------zone(${process.argv[2]}):worker(${process.argv[3]}) is initializing------`);
(<any>global).__in_napa = true;
(<any>global).__zone_id = process.argv[2];
(<any>global).__worker_id = process.argv[3];
require('../../');
console.log(`------zone(${process.argv[2]}):worker(${process.argv[3]}) is initialized------`);
var aliveInterval = 2000;
setInterval(function () {
    console.log(`------zone(${process.argv[2]}):worker(${process.argv[3]}) keep alive interval`, aliveInterval);
}, aliveInterval);
