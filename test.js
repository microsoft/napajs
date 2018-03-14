const napa = require('.');
let zone = napa.zone.create("myyyyyyyyzoneeeeeeeeeee", {workers:1});
console.log('~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~');
///console.log('--->>>1', module);
///console.log(require);
// console.log('$$$$$$$$$$$$$$$$$$$$$', napa.transport._registry);
// console.log('*********************', napa.transport._constructor_registry);
setTimeout(() => {
    console.log('xxxx', zone, '~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~timeout');
}, 15000);
//zone = napa.zone.node;
//zone.execute("testm", "func");
zone.execute(() => {
    //xxx;
    console.log(Buffer.alloc(100));
    console.log('........zone.execute.....');
    setTimeout(() => {console.log('.......setTimeout Callback.....');}, 100);
    console.log('........after setTimeout.....');
    //console.log(global.require);
    //console.log(Buffer.alloc(100));
    //console.log(require);
    //Buffer.alloc(10);
}).then((r) => {
    console.log('....................call back.................................');
});
/*
console.log(Buffer.alloc(30));
console.log(Buffer.from('./package.json'));
const fs = require('fs');
console.log('+++++++++++++++', napa.zone.current.id, fs, '+++++++++++++++++++');
fs.readFile('./package.json', (err, data) => {
    if (err) console.log(napa.zone.current.id, err);
    else console.log('<<<<<<<<<<<<<<<<<<<<<<<<<', napa.zone.current.id, data, '>>>>>>>>>>>>>>>>>>>');
});
//console.log(napa);
var zone;
try {
    zone = napa.zone.get('xxxxxxxxxxxxxxx');
    console.log('&&&&&&&&&get&&&&&&&&&&', napa.zone.current.id, '&&&&&&&&&&&&&&&&&&&&&&');
    zone.execute(() => {
        console.log('++++++++++++get++++++++++++++');
    }, []).then((res) => {
        console.log('*************get************');
    });
}
catch(e) {
    zone = napa.zone.create('xxxxxxxxxxxxxxx');
    console.log('&&&&&&&&&create&&&&&&&&&&', napa.zone.current.id, '&&&&&&&&&&&&&&&&&&&&&&');
    zone.execute(() => {
        console.log('+++++++++++++create+++++++++++++');
    }, []).then((res) => {
        console.log('************create*************');
    });
}
// let zone = napa.zone.node;
console.log('-----------------', JSON.stringify(zone), '-----------------');
*/
