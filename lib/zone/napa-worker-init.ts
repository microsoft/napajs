//console.log(require);
console.log('------A napa worker is initializing------');
(<any>global).__in_napa = true;
require('../../');
console.log('------A napa worker is initialized------');

let workerTimeout = 5000;
setTimeout(() => {
    console.log('------n napa-worker-init.js, issue a timeout of', workerTimeout);
}, workerTimeout);
