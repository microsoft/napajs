import * as zone from "./zone";

/// <summary> A virtual Zone consists only 1 worker, which is Node event loop. </summary>
export class NodeZone implements zone.Zone {
    
    public get id(): string {
        return "node";
    }

    public toJSON(): any {
        return { id: this.id, type: "node" };
    }

    public broadcast(arg1: any, arg2?: any) : Promise<zone.ResponseCode> {
        // TODO @asib: add implementation
        return undefined;
    }

    public broadcastSync(arg1: any, arg2?: any) : zone.ResponseCode {
        // TODO @asib: add implementation
        return undefined;
    }

    public execute(arg1: any, arg2: any, arg3?: any, arg4?: any) : Promise<zone.ExecuteResult> {
        // TODO @asib: add implementation
        return undefined;
    }

    public executeSync(arg1: any, arg2: any, arg3?: any, arg4?: any) : zone.ExecuteResult {
        // TODO @asib: add implementation
        return undefined;
    }
}
