// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

import * as napa from "../lib/index";
import {setImmediate, clearImmediate, setTimeout, clearTimeout, setInterval, clearInterval } from "timers";

// Run with "node --expose-gc misc/timers-memory.js" and observer node's memory usage.

export function allocateBigArray(): string[] {
    const kNumberOfStrings = 10000;
    const kRandomStringLength = 128;
    const kSlicePerString = Math.ceil(kRandomStringLength / 16);
    let a : string[] = new Array(kNumberOfStrings);

    for (let i = 0; i < kNumberOfStrings; ++i) {
        a[i] = Array(kSlicePerString + 1).join(
                    (Math.random().toString(16) + '0000000000000000').slice(2,18))
                .slice(0,kRandomStringLength);
    }
    return a;
}

function forceGC() {
    if (global.gc) {
        global.gc();
    }
    else {
        console.warn("No GC hook, Start program as node --expose-gc");
    }
}

let totalIterations = 0;
export function printLastStringAndRearm(id: number, ba: string[], ms: number): void {
    ++totalIterations;
    if (totalIterations % 100 == 0) {
        forceGC();
    }
    let nba = allocateBigArray();
    console.log(` ++++New add last element in array ${id} : ${nba[nba.length-1]}`);
    setTimeout(printLastStringAndRearm, ms, id, nba, ms);
}

export function timersMemoryTest() {
    for (let id = 0; id < 2; ++id) {
        let ba = allocateBigArray();
        console.log(` ----Initial last element in array ${id} : ${ba[ba.length-1]}`)
        setTimeout(printLastStringAndRearm, (id+1)*10, id, ba, 100);
    }
}


declare var __in_napa: boolean;
if (typeof __in_napa === 'undefined') {
    const NUMBER_OF_WORKERS = 1;
    let zone = napa.zone.create('zone', { workers: NUMBER_OF_WORKERS });
    zone.execute('./timers-memory', 'timersMemoryTest');

    //Run it in main node worker
    //timersMemoryTest();

    setTimeout(() => {
        console.log(" =========================");
    }, 100000000);
}

