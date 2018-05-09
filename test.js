const napa = require('.');

let napazone = napa.zone.create("myzone", {workers:1});

let nodezone = napa.zone.node;
let sab = new SharedArrayBuffer(4);
let ta = new Int8Array(sab);
ta[0] = 100;
napazone.execute((sharable, sab) => {
    console.log('...00 sharable...', sharable);
    let ta = new Int8Array(sab);
    console.log('...01 sharedArrayBuffer...', ta);
    ta[1] = 99;
    console.log('...0...zone.execute......', global.__zone_id, global.__worker_id);
    console.log('...1...Buffer......', Buffer.alloc(25));
    setTimeout(() => {
        console.log('...4...setTimeout Callback......', global.__zone_id, global.__worker_id);
    }, 100);
    console.log('...2...after setTimeout......');

    const napa = require('.');
    napa.zone.get('node').execute(()=>{
        console.log('......<<<inner zone.execute>>>......', global.__zone_id, global.__worker_id);
    }).then ((r)=>{
        console.log('......<<<inner zone.execute callback>>>......', global.__zone_id, global.__worker_id);
    })
    return ta;
}, [napa.memory.crtAllocator, sab]).then((r) => {
    console.log('...3...zone execute callback......', r.value, ta, global.__zone_id, global.__worker_id);
});
