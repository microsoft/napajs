var fs = require('fs');
var events = require('events');
var assert = require('assert');

process.env.NAPA_TEST = "process";
assert.equal(process.env.NAPA_TEST, "process");

assert(typeof process.platform != undefined && process.platform != null);

assert(typeof process.umask != undefined);
assert(process.umask() == process.umask(0));
assert(process.umask() == process.umask("0"));

try {
    process.chdir('./test');
    process.chdir(process.cwd());
    process.chdir('..');
    fs.existsSync('napa.dll');
} catch (err) {
    throw err;
}

assert(typeof process.pid != undefined && !isNaN(process.pid));

events.defaultMaxListeners = 1;
process.on('foo', () => {});
assert.throws(() => { process.on('foo', () => {})});

true;