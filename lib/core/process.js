// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

var binding = exports;

for (var prop in process) {
    binding[prop] = process[prop];
}

var EventEmitter = require('events').EventEmitter;
binding.__proto__ = Object.create(EventEmitter.prototype, { constructor : { value : binding.constructor } });
EventEmitter.call(binding);

binding.EventEmitter = EventEmitter;

var stdin = undefined;
var stdout = undefined;
var stderr = undefined;

function createWritableStdioStream(fd) {
    var tty = require('tty');
    var stream = new tty.WriteStream(fd);
    stream._type = 'tty';

    if (stream._handle && stream._handle.unref) {
        stream._handle.unref();
    }

    stream.fd = fd;
    stream._isStdio = true;

    return stream;
}

binding.__defineGetter__("stdin",
                         function() {
                             if (stdin) {
                                 return stdin;
                             }

                             var tty = require('tty');
                             stdin = new tty.ReadStream(0, { highWaterMark : 0, readable : true, writable : false });

                             stdin.fd = fd;

                             if (stdin._handle && stdin._handle.readStop) {
                                 stdin._handle.reading = false;
                                 stdin._readableState.reading = false;
                                 stdin._handle.readStop();
                             }

                             stdin.on('pause',
                                      function() {
                                          if (!stdin._handle) {
                                              return;
                                          }
                                          stdin._readableState.reading = false;
                                          stdin._handle.reading = false;
                                          stdin._handle.readStop();
                                      });

                             return stdin;
                         });

binding.__defineGetter__("stdout",
                         function() {
                             if (stdout) {
                                 return stdout;
                             }

                             stdout = createWritableStdioStream(1);
                             stdout.destroy = stdout.destroySoon = function(e) {
                                 e = e || new Error('process.stdout cannot be closed');
                                 stdout.emit('error', e);
                             };

                             if (stdout.isTTY) {
                                 process.on('SIGWINCH', function() { stdout._refreshSize(); });
                             }

                             return stdout;
                         });

binding.__defineGetter__("stderr",
                         function() {
                             if (stderr) {
                                 return stderr;
                             }

                             stderr = createWritableStdioStream(2);
                             stderr.destroy = stderr.destroySoon = function(e) {
                                 e = e || new Error('process.stderr cannot be closed');
                                 stderr.emit('error', e);
                             };

                             return stderr;
                         });