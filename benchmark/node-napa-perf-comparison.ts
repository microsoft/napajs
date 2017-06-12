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

export function bench(zone: napa.zone.Zone): Promise<void> {
    zone.broadcast(timeIt.toString());
    zone.broadcast(test1.toString());

    // Warm up.
    test1();
    zone.executeSync('', 'test1', []);

    // Actual test.
    let table = [];
    table.push(["node time", "napa time"]);
    table.push([
        formatTimeDiff(test1()), 
        formatTimeDiff(zone.executeSync('', 'test1', []).value)]);

    console.log("## Node vs Napa JavaScript execution performance\n");
    console.log(mdTable(table));
    console.log('');
    return new Promise<void>((resolve, reject) => {
        resolve();
    });
}