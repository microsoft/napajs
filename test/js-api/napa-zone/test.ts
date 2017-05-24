import * as napa from 'napajs';

export function bar(input: any) {
    return input;
}

export function getCurrentZone(): napa.zone.Zone {
    return napa.zone.current;
}

export function broadcast(id: string, code: string): void {
    let zone = napa.zone.get(id);
    // TODO: replace with broadcast when TODO:#3 is done.
    zone.broadcastSync(code);
}

export function broadcastTestFunction(id: string): void {
    // TODO: replace with broadcast when TODO:#3 is done.
    napa.zone.get(id).broadcastSync((input: string) => {
        console.log(input);
    }, ["hello world"]);
}

export function broadcastTransportable(id: string): void {
    // TODO: replace with broadcast when TODO:#3 is done.
    napa.zone.get(id).broadcastSync((input: any) => {
        console.log(input);
    }, [napa.memory.crtAllocator]);
}

export function broadcastClosure(id: string): void {
    let zone = napa.zone.get(id);
    // TODO: replace with broadcast when TODO:#3 is done.
    zone.broadcastSync(() => {
        console.log(zone);
    }, []);
}

export function execute(id: string, moduleName: string, functionName: string, args?: any[]): any {
    let zone = napa.zone.get(id);
    // TODO: replace with execute when TODO:#3 is done.
    return zone.executeSync(moduleName, functionName, args).value;
}

export function executeTestFunction(id: string): any {
    let zone = napa.zone.get(id);
    // TODO: replace with execute when TODO:#3 is done.
    return zone.executeSync((input: string) => {
            return input;
        }, ['hello world']);
}