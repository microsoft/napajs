import * as napa from 'napajs';
import * as mdTable from 'markdown-table';
import { formatTimeDiff } from './bench-utils';

export function bench(zone: napa.zone.Zone): Promise<void> {
    console.log("Benchmarking execute overhead...");

    // Prepare a empty function.
    zone.broadcastSync("function test() {}");
    const ARGS = [1, "hello", {a: 1, b: true}];

    // Warm-up.
    const WARMUP_REPEAT: number = 20;

    console.log("## Execute overhead during warmup\n")
    let warmupTable = [];
    warmupTable.push(["call #", "time (ms)"]);
    for (let i = 0; i < WARMUP_REPEAT; ++i) {
        let start = process.hrtime();
        zone.executeSync("", "test", ARGS);
        warmupTable.push([i.toString(), formatTimeDiff(process.hrtime(start))]);
    }
    console.log(mdTable(warmupTable));

    // executeSync after warm-up
    const REPEAT: number = 10000;
    console.log("## `zone.executeSync` overhead\n");
    let start = process.hrtime();
    for (let i = 0; i < REPEAT; ++i) {
        zone.executeSync("", "test", ARGS);
    }
    console.log(`Elapse of running empty function for ${REPEAT} times: ${formatTimeDiff(process.hrtime(start), true)}\n`);

    // execute after warm-up
    return new Promise<void>((resolve, reject) => {
        let finished = 0;
        let start = process.hrtime();
        for (let i = 0; i < REPEAT; ++i) {
            zone.execute("", "test", ARGS).then(() => {
                ++finished;
                if (finished === REPEAT) {
                    console.log("## `zone.execute` overhead\n");
                    console.log(`Elapse of running empty function for ${REPEAT} times: ${formatTimeDiff(process.hrtime(start), true)}`);
                    console.log('');
                    resolve();
                }
            });
        }
    });
}