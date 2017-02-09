var addon = require('../bin/addon');

export function createPlusNumberTransporter(value: number): any {
    return addon.createPlusNumberTransporter(value);
}

export function add(transporter: any, value: number): number {
    return addon.add(transporter, value);
}