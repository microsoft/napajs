//console.log(require);
console.log('-------------A napa worker is initializing--------------');
(<any>global).__in_napa = true;
//global.require = require;
require('../../');
//console.log('$$$$$$$$$$$$$$$$$$$$$', napa.transport._registry);
//console.log('*********************', napa.transport._constructor_registry);
//console.log(require);
console.log('-------------A napa worker is initialized--------------');
//console.log(global.require);
console.log(Buffer.alloc(30));
///console.log('--->>>', module);
//global.require = require;
//global.Buffer = Buffer;
//console.log(global);
let workerTimeout = 25000;
setTimeout(() => {
    console.log('In napa-worker-init.js, issue a timeout of', workerTimeout);
}, workerTimeout);
