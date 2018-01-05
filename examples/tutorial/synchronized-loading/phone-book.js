// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

const napa = require("napajs");
const store = napa.store.getOrCreate('my-phone-book');

const initialize = function () {
    store.set('_loadLock', napa.sync.createLock());
}

const load_data = function () {
    // load data. This function should run only once.
    console.log('[load_data] loading...');
    
    const fs = require('fs');
    let phoneBookData = JSON.parse(fs.readFileSync('./phone-book-data.json').toString());

    for (let name in phoneBookData) {
        store.set(name, phoneBookData[name]);
    }
    store.set('_loaded', true);
}

let loaded = false;
const lookup = function (name) {
    if (!loaded) {
        const lock = store.get('_loadLock');
        lock.guardSync(function() {
            if (!store.get('_loaded')) {
                load_data();
            }
        });
        loaded = true;
    }

    return store.get(name);
};

module.exports.initialize = initialize;
module.exports.lookup = lookup;
