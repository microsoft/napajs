// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

import * as napa from 'napajs';
import * as mdTable from 'markdown-table';
import { formatTimeDiff } from './bench-utils';

function batchExecuteOnNamedFunction(
    zone: napa.zone.Zone, 
    repeat: number,
    args: any[]): Promise<void> {
    
    return new Promise<void>((resolve, reject) => {
        let finished = 0;
        let start = process.hrtime();
        for (let i = 0; i < repeat; ++i) {
            zone.execute("", "test", args).then(() => {
                ++finished;
                if (finished === repeat) {
                    resolve();
                }
            });
        }
    });
}

function batchExecuteOnAnonymousFunction(
    zone: napa.zone.Zone, 
    repeat: number,
    args: any[]): Promise<void> {
    
    return new Promise<void>((resolve, reject) => {
        let finished = 0;
        let start = process.hrtime();
        for (let i = 0; i < repeat; ++i) {
            zone.execute(() => {}, args).then(() => {
                ++finished;
                if (finished === repeat) {
                    resolve();
                }
            });
        }
    });
}

export async function bench(zone: napa.zone.Zone): Promise<void> {
    console.log("Benchmarking execute overhead...");

    // Prepare a empty function.
    await zone.broadcast("function test() {}");
    const ARGS = [1, "hello", {a: 1, b: true}];

    // Warm-up.
    const WARMUP_REPEAT: number = 20;

    console.log("## Execute overhead during warmup\n")
    let warmupTable = [];
    warmupTable.push(["call #", "time (ms)"]);
    for (let i = 0; i < WARMUP_REPEAT; ++i) {
        let start = process.hrtime();
        await zone.execute("", "test", ARGS);
        warmupTable.push([i.toString(), formatTimeDiff(process.hrtime(start))]);
    }
    console.log(mdTable(warmupTable));

    // execute after warm-up
    const REPEAT: number = 10000;
    console.log("## `zone.execute` overhead (use function name)\n");
    let start = process.hrtime();
    await batchExecuteOnNamedFunction(zone, REPEAT, ARGS);
    console.log(`Elapse of running empty function by name for ${REPEAT} times: ${formatTimeDiff(process.hrtime(start), true)}\n`);

    console.log("## `zone.execute` overhead (use anonymous function)\n");
    start = process.hrtime();
    await batchExecuteOnAnonymousFunction(zone, REPEAT, ARGS);
    console.log(`Elapse of running empty anonymous function for ${REPEAT} times: ${formatTimeDiff(process.hrtime(start), true)}\n`);

    return;
}