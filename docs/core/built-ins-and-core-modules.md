# Napa.JS Built-ins and Core Modules

## Node.JS compatible APIs
- fs
    - fs.readFileSync (SUPPORTED)
    - fs.writeFileSync (SUPPORTED)
    - others (TBD) 
- path (consider use Node.JS' implementation)
- events
    - Class: EventEmitter
- stream (TBD)
- assert (TBD)
    - assert(value[, message])
    - assert.deepEqual
    - assert.deepStrictEqual
    - assert.doesNotThrow
    - assert.equal
    - assert.fail
    - assert.ifError
    - assert.notDeepEqual
    - assert.notDeepStrictEqual
    - assert.notEqual
    - assert.notStrictEqual
    - assert.ok
    - assert.strictEqual
    - assert.throws
- Global Objects
    - Class: Buffer (TBD)
    - __dirname (SUPPORTED)
    - __pathname (SUPPORTED)
    - console
        - console.assert
        - console.dir
        - console.error
        - console.info
        - console.log (SUPPORTED)
        - console.time
        - console.timeEnd
        - console.trace
        - console.warn
    - exports
    - global (TBD)
    - module
    - process (TBD)
    - require
    - setImmediate (TBD)
    - setInterval (TBD)
    - setTimeout (TBD)


## Napa specific APIs

- global readonly variable '*__in_napa_container*': Tell if current code is running under a Napa.JS container. 