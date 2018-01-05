// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

const napa = require("napajs");

// Change this value to control number of napa workers initialized.
const NUMBER_OF_WORKERS = 4;

// Create a napa zone with number_of_workers napa workers.
const zone = napa.zone.create('zone', { workers: NUMBER_OF_WORKERS });

function run() {
    // Initialize the phone book component
    const phoneBook = require('./phone-book');
    phoneBook.initialize();
    
    // Setup all workers with functions to be executed.
    zone.broadcast(' \
        var napa = require("napajs"); \
        var zone = napa.zone.get("zone"); \
        var phoneBook = require("./phone-book"); \
    ');
    zone.broadcast(lookupPhoneNumber.toString());

    var tasks = [];
    tasks.push(zone.execute('', 'lookupPhoneNumber', ['david']));
    tasks.push(zone.execute('', 'lookupPhoneNumber', ['lisa']));
    tasks.push(zone.execute('', 'lookupPhoneNumber', ['wade']));

    Promise.all(tasks).then(function () {
        console.log('[run] All operations are completed.');
    });
}

function lookupPhoneNumber(name) {
    console.log(`[lookupPhoneNumber] Start to lookup phone number of ${name}.`);
    var n = phoneBook.lookup(name);
    console.log(`[lookupPhoneNumber] ${name} : ${n ? n : '<not found>'}.`);
}

// Run program.
run();
