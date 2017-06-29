import * as napa from 'napajs';
import * as mdTable from 'markdown-table';
import { formatTimeDiff } from './bench-utils';

export function timeIt(func: () => void): [number, number] {
    let start = process.hrtime();
    func();
    return process.hrtime(start);
}

export function test1(): [number, number] {
    return timeIt(() => {
        const REPEAT = 1000000000;
        let sum = 0;
        for (let i = 0; i < REPEAT; ++i) {
            sum += i;
        }
    });
}

export async function bench(zone: napa.zone.Zone): Promise<void> {
    zone.broadcast(timeIt.toString());
    zone.broadcast(test1.toString());

    // Warm up.
    test1();
    await zone.execute('', 'test1', []);

    // Actual test.
    let table = [];
    table.push(["node time", "napa time"]);
    let nodeTime = formatTimeDiff(test1());
    let napaTime = formatTimeDiff((await zone.execute('', 'test1', [])).value);
    table.push([nodeTime, napaTime]);
        
    console.log("## Node vs Napa JavaScript execution performance\n");
    console.log(mdTable(table));
    console.log('');
}