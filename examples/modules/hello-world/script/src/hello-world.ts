var addon = require('../bin/addon');

export function hello(): string {
    return addon.hello();
}