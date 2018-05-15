# Napa.js vNext node API compatibility

This file is used to track node API compatiblity.

| module            | function                             | compatibility | remarks                                    |
|-------------------|--------------------------------------|---------------|--------------------------------------------|
| assert            | *                                    | o             |
| Buffer            | Buffer.alloc()                       | o             |
| Buffer            | Buffer.read()                        | x             | stream is not multi-thread ready

**TBD: The above is an example. should fill this table in future versions. free to add column if necessary.