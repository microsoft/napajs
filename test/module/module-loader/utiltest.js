const assert = require('assert');
const util = require('util');
const EventEmitter = require('events');

function MyStream() {
    EventEmitter.call(this);
}

util.inherits(MyStream, EventEmitter);

MyStream.prototype.write = function(data) {
    this.emit('data', data);
};

const stream = new MyStream();

assert(stream instanceof EventEmitter);
assert.equal(MyStream.super_, EventEmitter);

stream.on('data', (data) => {
    assert.equal(data, 'test');
});
stream.write('test');

var inspect = util.inspect(stream.write);
assert.equal(inspect, '[Function]');

assert.equal(util.format('%s:%s', 'foo', 'bar', 'baz'), 'foo:bar baz');

true;
