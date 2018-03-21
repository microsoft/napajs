const napa = require('.');
let zone = napa.zone.create("myzone", {workers:1});
// console.log('$$$$$$$$$$$$$$$$$$$$$', napa.transport._registry);
// console.log('*********************', napa.transport._constructor_registry);
setTimeout(() => {
    console.log('...........node setTimeput callback...........');
}, 1000);
//zone = napa.zone.node;
zone.execute(() => {
    console.log('......zone.execute......');
    console.log('......Buffer......', Buffer.alloc(25));
    setTimeout(() => {
        console.log('......setTimeout Callback......');
    }, 100);
    console.log('......after setTimeout......');

    const napa = require('.');
    napa.zone.get('myzone').execute(()=>{
        console.log('<<<inner zone.execute>>>');
    }).then ((r)=>{
        console.log('<<<inner zone.execute callback>>>');
    })

    return 123;
}).then((r) => {
    console.log('......zone execute callback......', r.value);
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
