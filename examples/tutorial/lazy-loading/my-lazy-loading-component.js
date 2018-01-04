// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

const napa = require("napajs");
const store = napa.store.getOrCreate('my-lazy-loading-component');

const initialize = function () {
    store.set('_lock', napa.sync.createLock());
}

const load_data = function () {
    // load data. This function should run only once.
    console.log('[load_data]loading...');
    
    const fs = require('fs');
    let shared_const_data = {
        'data1': fs.readFileSync('./data1').toString(),
        'data2': fs.readFileSync('./data2').toString()
    };

    for (let i in shared_const_data) {
        store.set(i, shared_const_data[i]);
    }
    store.set('_loaded', true);
}

let loaded = false;
const do_something = function (key) {
    console.log(`[do_something]key=${key.toString()}, trying to load data...`);

    if (!loaded) {
        const lock = store.get('_lock');
        lock.guardSync(function() {
            if (!store.get('_loaded')) {
                load_data();
            }
        });
        loaded = true;
    }

    // use data
    let data = store.get(key);
    console.log(`[do_something]key=${key.toString()}, data=${data ? data : '<not exist>'}`);
};

module.exports.initialize = initialize;
module.exports.do_something = do_something;
