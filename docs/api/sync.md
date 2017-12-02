# Namespace `sync`
## Table of Contents
- class [`Lock`](#class-lock)
    - [`lock.guardSync(func: (...params: any[]) => any, params?: any[]): any`](#lock-guard-sync-func-any-any)
<!--
    // Preserve this interface for async lock.
    - [`lock.guard(func: () => Promise<any>): Promise<any>`](#lock-guard-func-promise-any-promise-any)
-->
<!--
- class [`ReadWriteLock`](#class-readwritelock)
    - [`rwlock.guardRead(func: () => any): any`](#rwlock-guardread-func-any-any)
    - [`rwlock.guardWrite(func: () => any): any`](#rwlock-guardwrite-func-any-any)
-->

## APIs
Namespace `sync` deal with synchronization between threads in Napa. `Lock` <!-- and `ReadWriteLock` are --> is provided for exclusive and shared mutex scenarios.
## <a name="class-lock"></a> Interface `Lock`
Exclusive Lock, which is [transportable](transport.md#transportable) across JavaScript threads.

Use `napa.sync.createLock()` to create a lock.
```ts
var lock = napa.sync.createLock();
```
### <a name="lock-guard-sync-func-any-any"></a> lock.guardSync(func: (...params: any[]) => any, params?: any[]): any
Run input function synchronously and obtain the lock during its execution, returns what the function returns, or throws error if input function throws. Lock will be released once execution finishes.
```ts
try {
    var value = lock.guardSync(() => {
        // DoSomething may throw.
        return DoSomething();
    });
    console.log(value);
}
catch(error) {
    console.log(error);
}
```
<!--
### <a name="lock-guard-func-promise-any-promise-any"></a> lock.guard(func: () => Promise\<any>): Promise\<any>

// TBD: this is a non-blocking guard function.
//      It tries to obtain the lock and run the func.
//      It will return regardless of whether it obtained the lock successfully.

-->
<!--
## Class `ReadWriteLock`
Read-write lock, which is [transportable](transport.md#transportable) across JavaScript threads.

// TBD: RWLock APIs may be redesign in future.

Example: Creating a read-write lock with operator `new`.
```ts
let lock = new napa.sync.ReadWriteLock();
```

### rwlock.guardRead(func: () => any): any
Run input function synchronously and obtain the read (shared) lock during its execution, returns what the function returns, or throws error if input function throws. Read lock will be released once execution finishes. Multiple guardRead across threads can enter simultaneously while no pending guardWrite.

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

Run input function synchronously and obtain the write (exclusive) lock during its execution, returns what the function returns, or throws error if input function throws. Write lock will be released once execution finishes. Multiple guardWrite across threads cannot entered simultaneously, and will always wait if there is any pending guardRead.

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
-->
