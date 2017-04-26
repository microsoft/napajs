import * as zone from "./zone";

/// <summary> A virtual Zone consists only 1 worker, which is Node event loop. </summary>
export class NodeZone implements zone.Zone {
    
    public get id(): string {
        return "node";
    }

    public broadcast(source: string) : Promise<zone.ResponseCode> {
        // TODO @asib: add implementation
        return undefined;
    }

    public execute(module: string, func: string, args: string[], timeout?: number) : Promise<zone.ResponseValue> {
        // TODO @asib: add implementation
        return undefined;
    }
}
