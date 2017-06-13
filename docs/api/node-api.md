# Node.JS Compatibility

Napa doesn't support full compatibility with node.js and necessary core modules will be added incrementally. Here are the list of what Napa currently supports. Please, refer to https://nodejs.org/api/all.html for details.

## Assert

Since Napa doesn't suport *Buffer* yet, assert is not working on *Buffer*.

* assert(value[, message])
* assert.deepEqual(actual, expected[, message])
* assert.doesNotThrow(block[, error][, message])
* assert.equal(actual, expected[, message])
* assert.fail(actual, expected, message, operator)
* assert.ifError(value)
* assert.notDeepEqual(actual, expected[, message])
* assert.notEqual(actual, expected[, message])
* assert.notStrictEqual(actual, expected[, message])
* assert.ok(value[, message])
* assert.strictEqual(actual, expected[, message])
* assert.throws(block[, error][, message])

## Console

* console.log([data][, ...args])

## Events

* Event: 'newListener'
* Event: 'removeListener'
* EventEmitter.listenerCount(emitter, eventName)
* EventEmitter.defaultMaxListeners
* emitter.addListener(eventName, listener)
* emitter.emit(eventName[, ...args])
* emitter.eventNames()
* emitter.getMaxListeners()
* emitter.listenerCount(eventName)
* emitter.listeners(eventName)
* emitter.on(eventName, listener)
* emitter.once(eventName, listener)
* emitter.prependListener(eventName, listener)
* emitter.prependOnceListener(eventName, listener)
* emitter.removeAllListeners([eventName])
* emitter.removeListener(eventName, listener)
* emitter.setMaxListeners(n)

## File system

* fs.readFileSync(path)
* fs.writeFileSync(file, data)
* fs.mkdirSync(path)
* fs.existsSync(path)
* fs.readdirSync(path)

## Globals

* __dirname
* __filename
* console
* exports
* global
* module
    * module.exports
    * module.id
* process
* require

## OS

* os.type

## Path

* path.basename(path[, ext])
* path.dirname(path)
* path.extname(path)
* path.format(pathObject)
* path.isAbsolute(path)
* path.join([...paths])
* path.normalize(path)
* path.relative(from, to)
* path.resolve([...paths])
* path.sep

## Prcess

* process.argv
* process.cwd()
* process.chdir(directory)
* process.env
* process.execPath
* process.exit(code)
* process.hrtime([time])
* process.pid
* process.platform
* process.umask([mask])

## TTY

* tty.isatty(fd)

## Util

* util.debuglog(section)
* util.format(format[, ...args])
* util.inherits(constructor, superConstructor)
* util.inspect(object[, options])
* util._extend(target, source)