var addon = require('../bin/addon');

export function increase(extra: number, callback: (now: number) => void) {
    return addon.increase(extra, callback);
}

export function now(): string {
    return addon.now();
}
