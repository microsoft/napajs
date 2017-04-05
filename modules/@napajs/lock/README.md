# @napajs/lock - Lock for thread synchronization under NapaJS


## API
### Simple Lock
#### API
```ts
guard(lockName: string, func: {(): any}): any;
```
Guard execution of a function with a lock name. Return value from the function will be relayed.
Exception thrown in the function will be propagate to external.

#### Example
```ts
import * as lock from "@napajs/locks"
import * as assert from "assert"

{
    let returnValue = lock.guard('lockName', () => {
        // Do something here.
        return [1, 2, 3];
    });

    assert.deepEqual(returnValue, [1, 2, 3]);
}
{
    try {
        lock.guard('lockName', () => {
            // Exception is thrown.
            throw new Error();
        });
    }
    catch (e) {
        console.log(e);
    }
}

```
### Read-write lock
#### API
```ts
guardRead(lockName: string, func: {(): any}): any;
```
Guard execution of a function as a reader. That is, multiple readers can obtain the lock at the same time without blocking, if lock is not obtained by the writer. 

Return value from the function will be relayed. Exception thrown in the function will be propagate to external.

```ts
guardWrite(lockName: string, func: {(): any}): any;
```
Guard execution of a function as a writer. That is, only one writer can obtain the lock, all other readers and writers will wait. 

Return value from the function will be relayed. Exception thrown in the function will be propagate to external.

#### Examples

```ts
import * as lock from "@napajs/locks"
import * as assert from "assert"

{
    let returnValue = lock.guardRead('lockName', () => {
        // Do read here.
        return "hello-world";
    });
    assert.equal(returnValue, "hello-world");
}

// Handle exceptions thrown from input functions.
{
    let succeeded: boolean = false;
    
    // Do some work...
    try {
        let returnValue = lock.guardWrite('lockName', () => {
            // Do write here.
            if (!succeeded)
                throw new Error();

            return 123;
        });
        assert.equal(returnValue, 123);
    }
    catch (e) {
        console.log(e);
    }
}
```

