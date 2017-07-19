// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

import * as napa from '../lib/index';
import * as assert from 'assert';
import * as mdTable from 'markdown-table';
import { generateObject, formatTimeDiff } from './bench-utils';

type BenchmarkSettings = [
    string,     // label
    number,     // # of keys per level
    number,     // # of levels
    string,     // value type
    number      // value length for string.
];

export function bench() {
    console.log("Benchmarking store overhead...");
    let settings: BenchmarkSettings[] = [
        // Number
        ["1 level - 1 integers", 1, 1, "number", 0],
        ["1 level - 10 integers", 10, 1, "number", 0],
        ["1 level - 100 integers", 100, 1, "number", 0],
        ["10 level - 2 integers", 2, 10, "number", 0],
        ["2 level - 10 integers", 10, 2, "number", 0],
        ["3 level - 5 integers", 5, 3, "number", 0],

        // String
        ["1 level - 10 strings - length 10", 10, 1, "string", 10],
        ["1 level - 100 strings - length 10", 100, 1, "string", 10],
        ["2 level - 10 strings - length 10", 10, 2, "string", 10],
        ["3 level - 5 strings - length 10", 5, 3, "string", 10],

        ["1 level - 10 strings - length 100", 10, 1, "string", 100],
        ["1 level - 100 strings - length 100", 100, 1, "string", 100],
        ["2 level - 10 strings - length 100", 10, 2, "string", 100],
        ["3 level - 5 integers", 5, 3, "string", 100],

        // Boolean
        ["1 level - 10 booleans", 10, 1, "boolean", 0],
        ["1 level - 100 booleans", 100, 1, "boolean", 0],
        ["2 level - 10 booleans", 10, 2, "boolean", 0],
        ["3 level - 5 booleans", 5, 3, "boolean", 0],
    ];

    let store = napa.store.create('store');
    let table = [];
    table.push(["payload type", "size", "transport.marshall (ms)", "store.save (ms)", "transport.unmarshall (ms)", "store.get (ms)"]);

    for (let s of settings) {
        const REPEAT = 1000;
        let object = generateObject(s[1], s[2], s[3], s[4])
        let payload = JSON.stringify(object);
        let size = payload.length;

        // transport.marshall
        let start = process.hrtime();
        for (let i = 0; i < REPEAT; ++i) {
            napa.transport.marshall(object, null);
        }
        let marshallTime = formatTimeDiff(process.hrtime(start));

        // store.set
        start = process.hrtime();
        for (let i = 0; i < REPEAT; ++i) {
            store.set('key', object);
        }
        let storeSetTime = formatTimeDiff(process.hrtime(start));

        assert.deepEqual(object, store.get('key'));

        // transport.unmarshall
        start = process.hrtime();
        for (let i = 0; i < REPEAT; ++i) {
            napa.transport.unmarshall(payload, null);
        }
        let unmarshallTime = formatTimeDiff(process.hrtime(start));

        // store.get
        start = process.hrtime();
        for (let i = 0; i < REPEAT; ++i) {
            store.get('key');
        }
        let storeGetTime = formatTimeDiff(process.hrtime(start));

        table.push([s[0], size, marshallTime, storeSetTime, unmarshallTime, storeGetTime]);
    }
    console.log("## Store access overhead\n");
    console.log(mdTable(table));
    console.log('');
}