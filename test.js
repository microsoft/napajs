const napa = require('.');
let zone = napa.zone.create("myzone", {workers:1});
// console.log('$$$$$$$$$$$$$$$$$$$$$', napa.transport._registry);
// console.log('*********************', napa.transport._constructor_registry);
var nodeTimeout = 1000;
setTimeout(() => {
    console.log('------node setTimeput callback------', nodeTimeout);
}, nodeTimeout);

// zone = napa.zone.node;
zone.execute(() => {
    console.log('...0...zone.execute......', global.__zone_id, global.__worker_id);
    console.log('...1...Buffer......', Buffer.alloc(25));
    setTimeout(() => {
        console.log('...4...setTimeout Callback......', global.__zone_id, global.__worker_id);
    }, 100);
    console.log('...2...after setTimeout......');

    const napa = require('.');
    napa.zone.get('myzone').execute(()=>{
        console.log('......<<<inner zone.execute>>>......', global.__zone_id, global.__worker_id);
    }).then ((r)=>{
        console.log('......<<<inner zone.execute callback>>>......', global.__zone_id, global.__worker_id);
    })

    return 123;
}).then((r) => {
    console.log('...3...zone execute callback......', r.value, global.__zone_id, global.__worker_id);
});
/*exports.foo = () => {
    //xxx;
    console.log(Buffer.alloc(100));
    console.log('........zone.execute.....');
    setTimeout(() => {console.log('.......setTimeout Callback.....');}, 100);
    console.log('........after setTimeout.....');
    //console.log(global.require);
    //console.log(Buffer.alloc(100));
    //console.log(require);
    //Buffer.alloc(10);
};
zone.execute('test', 'foo').then((r) => {
    console.log('....................call back.................................');
});
*/
