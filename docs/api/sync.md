# namespace `sync`
## Table of Contents
- class [`Lock`](#class-lock)
    - [`lock.guard(func: () => any): any`](#lock-guard-func-any-any)
- class [`ReadWriteLock`](#class-readwritelock)
    - [`rwlock.guardRead(func: () => any): any`](#rwlock-guardread-func-any-any)
    - [`rwlock.guardWrite(func: () => any): any`](#rwlock-guardwrite-func-any-any)

## APIs
Namespace `sync` deal with synchronization between threads in Napa. `Lock` and `ReadWriteLock` are provided for exclusive and shared mutex scenarios.
## Class `Lock`
Exclusive Lock, which is [transportable](transport.md#transportable) across JavaScript threads.

Example: Creating a lock with operator `new`.
```ts
let lock = new napa.sync.Lock();
```
### lock.guard(func: () => any): any
Run input function synchronously and obtain the lock during its execution, returns what the function returns, or throws error if input function throws. Lock will be released once execution finishes.
```ts
try {
    let value = lock.guard(() => {
        // DoSomething may throw.
        return DoSomething();
    });
    console.log(value);
}
catch(error) {
    console.log(error);
}
```
## Class `ReadWriteLock`
Read-write lock, which is [transportable](transport.md#transportable) across JavaScript threads.

Example: Creating a read-write lock with operator `new`.
```ts
let lock = new napa.sync.ReadWriteLock();
```

### rwlock.guardRead(func: () => any): any
Run input function synchronously and obtain the read (shared) lock during its execution, returns what the function returns, or throws error if input function throws. Read lock will be released once execution finishes. Multiple guardRead across threads can enter simutenously while no pending guardWrite.

```ts
try {
    let value = lock.guardRead(() => {
        // DoRead may throw.
        return DoRead();
    });
    console.log(value);
}
catch(error) {
    console.log(error);
}
```

### rwlock.guardWrite(func: () => any): any

Run input function synchronously and obtain the write (exclusive) lock during its execution, returns what the function returns, or throws error if input function throws. Write lock will be released once execution finishes. Multiple guardWrite across threads cannot entered simutenously, and will always wait if there is any pending guardRead.

```ts
try {
    let value = lock.guardWrite(() => {
        // DoWrite may throw.
        return DoWrite();
    });
    console.log(value);
}
catch(error) {
    console.log(error);
}