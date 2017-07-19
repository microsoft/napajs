// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

import * as napa from '../lib/index';
import * as nodeNapaPerfComp from './node-napa-perf-comparison';
import * as executeOverhead from './execute-overhead';
import * as executeScalability from './execute-scalability';
import * as transportOverhead from './transport-overhead';
import * as storeOverhead from './store-overhead';

export function bench(): Promise<void> {
    // Non-zone related benchmarks.
    transportOverhead.bench();
    storeOverhead.bench();

    // Create zones for execute related benchmark.
    let singleWorkerZone = napa.zone.create('single-worker-zone', { workers: 1});
    let multiWorkerZone = napa.zone.create('multi-worker-zone', { workers: 8 });

    return nodeNapaPerfComp.bench(singleWorkerZone)
        .then(() => { return executeOverhead.bench(singleWorkerZone); })
        .then(() => { return executeScalability.bench(multiWorkerZone);});
}

bench();