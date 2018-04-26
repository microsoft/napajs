const __in_napa: boolean = true;
(<any>global).__in_napa = __in_napa;

const __zone_id: string = process.argv[2];
(<any>global).__zone_id = __zone_id;

const __worker_id = process.argv[3];
(<any>global).__worker_id = __worker_id;

console.log(`------zone(${__zone_id}):worker(${__worker_id}) is initializing------`);
require('../../');
console.log(`------zone(${__zone_id}):worker(${__worker_id}) is initialized------`);

var aliveInterval = 30000;
(<any>global).__recycle = (function () {
    let handle = setInterval(function () {
        console.log(`------zone(${__zone_id}):worker(${__worker_id}) keep alive interval at ${new Date().toISOString()}`);
    }, aliveInterval);

    return function () {
        clearInterval(handle);
        console.log(`------zone(${__zone_id}):worker(${__worker_id}) is terminating------`);
    };
})();
