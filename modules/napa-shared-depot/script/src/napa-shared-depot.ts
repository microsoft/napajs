var addon = require('../bin/addon');

export declare class SharedBarrel {
    public id(): number;
    public exists(key: string): boolean;
    public set(key: string, object: any): boolean;
    public get(key: string): any;
    public remove(key:string): void;
    public count(): number;
}

export function assign(): SharedBarrel {
    return addon.assign();
}

export function release(barrel: number | SharedBarrel) {
    return addon.release(barrel);
}

export function find(id: number): SharedBarrel {
    return addon.find(id);
}

export function count(): number {
    return addon.count();
}
