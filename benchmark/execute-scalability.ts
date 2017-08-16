// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

import * as napa from '../lib/index';
import * as assert from 'assert';
import * as mdTable from 'markdown-table';
import {formatTimeDiff} from './bench-utils';

function makeCRCTable() {
    var c;
    var crcTable = [];
    for (var n = 0; n < 256; n++) {
        c = n;
        for (var k = 0; k < 8; k++) {
            c = ((c & 1) ? (0xEDB88320 ^ (c >>> 1)) : (c >>> 1));
        }
        crcTable[n] = c;
    }
    return crcTable;
}

let crcTable = makeCRCTable();
function crc32(str: string) {
    let crc = 0 ^ (-1);
    for (var i = 0; i < str.length; i++) {
        crc = (crc >>> 8) ^ crcTable[(crc ^ str.charCodeAt(i)) & 0xFF];
    }

    return (crc ^ (-1)) >>> 0;
};

function testCrc() {
    const REPEAT: number = 1000000;
    let result = 0;
    let key = Array(1024).join('x');
    for (let i = 0; i < REPEAT; ++i) {
        let hash = crc32(key);
        result = result ^ hash;
    }
    return result;
}

export async function bench(zone: napa.zone.Zone): Promise<void> {
    console.log("Benchmarking execute scalability...");

    // Prepare a empty function.
    await zone.broadcast(makeCRCTable.toString());
    await zone.broadcast("var crcTable = makeCRCTable();");
    await zone.broadcast(crc32.toString());
    await zone.broadcast(testCrc.toString());

    // Warm-up.
    let crcResult = testCrc();
    await zone.broadcast("testCrc()");

    // Execute in Node with 1 thread.
    let start = process.hrtime();
    assert(testCrc() === crcResult);
    let nodeTime = formatTimeDiff(process.hrtime(start));

    let executeTime = {};
    let scalabilityTest = function(workers: number): Promise<void> {
        let finished = 0;
        let start = process.hrtime();

        return new Promise<void>((resolve, reject) => {
            for (let i = 0; i < workers; ++i) {
                zone.execute("", "testCrc", [])
                    .then((result: napa.zone.Result) => {
                        assert(crcResult === result.value);
                        ++finished;
                        if (finished === workers) {
                            executeTime[workers] = formatTimeDiff(process.hrtime(start)) resolve();
                        }
                    });
            }
        })
    };

    // Execute from 1 worker to 8 workers.
    await scalabilityTest(1);
    await scalabilityTest(2);
    await scalabilityTest(4);
    await scalabilityTest(8);
    console.log("## Execute scalability\n") console.log(mdTable([
        [ "node", "napa - 1 worker", "napa - 2 workers", "napa - 4 workers", "napa - 8 workers" ],
        [ nodeTime, executeTime[1], executeTime[2], executeTime[4], executeTime[8] ]
    ]));
    console.log('');
}