// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

const napa = require("napajs");

// Change this value to control number of napa workers initialized.
const NUMBER_OF_WORKERS = 3;

// Create a napa zone with number_of_workers napa workers.
const zone = napa.zone.create('zone', { workers: NUMBER_OF_WORKERS });

function run() {
    // Initialize the component
    const component = require('./my-lazy-loading-component');
    component.initialize();
    
    // Setup all workers with functions to be executed.
    zone.broadcast(' \
        var napa = require("napajs"); \
        var zone = napa.zone.get("zone"); \
        var component = require("./my-lazy-loading-component"); \
    ');
    zone.broadcast(use_lazy_loading_component.toString());

    var tasks = [];
    tasks.push(zone.execute('', 'use_lazy_loading_component', ['data1']));
    tasks.push(zone.execute('', 'use_lazy_loading_component', ['data2']));
    tasks.push(zone.execute('', 'use_lazy_loading_component', ['data3']));

    Promise.all(tasks).then(function () {
        console.log('[run]All operations are completed.');
    });
}

function use_lazy_loading_component(key) {
    global.component.do_something(key);
}

// Run program.
run();
