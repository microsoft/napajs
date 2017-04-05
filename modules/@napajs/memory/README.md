# @napajs/memory: Handling memory in NapaJS

## Working with native objects

Interface for NativeObject, as a convention, to enable us to provide common facilities for native objects. A readonly property `handle` is the only obligation for now.

```ts
/// <summary> Handle defines minimum information to access a native object. 
/// Property 'type' is a string to identify the object type, which can be used for debugging and type checking in JavaScript.
/// Property 'pointer' represents a 64-bit pointer in a 2-element number array, pointer[0] is the higher 32-bit, and pointer[1] is the lower 32-bit.
/// </summary>
type Handle = { type: string, pointer: [number, number]};

/// <summary> Interface for all native object. </summary>
interface NativeObject {
    readonly handle: Handle;
}

```
## Sharing object across isolates
- Provide facilities for sharing objects across isolates at the global scope or a local scope (like within a request).
- Support both plain JS objects and wrappers for native objects

### API
```ts
/// <summary> Barrel defines a scope for sharing objects. </summary>
declare class Barrel {
    /// <summary> Get id for current barrel. </summary>
    readonly id: string;

    /// <summary> Set a shared object. </summary>
    set(key: string, value: any);

    /// <summary> Get a shared object by key. </summary>
    get(key: string): any;

    /// <summary> Check if a key exists. </summary>
    exists(key: string): boolean;

    /// <summary> Remove a shared object by key. </summary>
    remove(key: string): void;

    /// <summary> Release a barrel. </summary>
    release(): void;

    /// <summary> Get number of objects shared in current barrel. </summary>
    count(): number:
}

/// <summary> Create a new barrel for sharing. </summary>
declare function createBarrel() : Barrel;

/// <summary> Find an existing barrel by ID </summary>
/// <returns> A Barrel object, if not found, exception will be thrown. </returns>
declare function findBarrel(id) : Barrel;

/// <summary> Returns number of barrels. </summary>
declare function barrelCount(): number;

/// <summary> Barrel for sharing at global namespace. </summary>
declare var global: Barrel;

```
### Examples:
#### Sharing JS objects at global namespace.

```ts
let mem = require('@napajs/memory');

mem.global.set('sample', {
    foo: 1,
    bar: "hello world"
});

/// In another isolate.
assert(mem.global.exists('sample'));
let object = mem.global.get('sample');
assert.deepEqual(object, {
    foo: 1,
    bar: "hello world"
})
```
#### Sharing JS objects at request level.
```ts
let mem = require('@napajs/memory');

let barrel = mem.createBarrel());

barrel.set('sample', {
    foo: 1,
    bar: "hello world"
});

/// In another isolate.
declare var id: string;
let barrel = mem.findBarrel(id);
assert(barrel.exists('sample'));
let object = barrel.get('sample');
assert.deepEqual(object, {
    foo: 1,
    bar: "hello world"
})

// Release barrel at end of request.
barrel.release();
```
#### Sharing native objects
TODO: move shared-depot examples here.

## Allocator support
- Create customized allocator
- Pass allocators across isolates
- Consume allocator in C++
- Consume allocator in JavaScript (optional)

### API
#### JS interface (Optional)
```ts
/// <summary> 
interface Allocator : NativeObject {
    /// <summary> Allocate memory of requested size. </summary>
    allocate(size: number): Buffer

    /// <summary> Free allocated memory. </summary>
    free(buffer: Buffer): void;

    /// <summary> Handle of this allocator. </summary>
    readonly handle: Handle;

    /// <summary> Release current allocator. </summary>
    release(): void;
}

interface AllocatorFactory {
    create(allocatorType: string): Allocator;
}

/// <summary> add an allocator factory. </summary>
declare function registerAllocatorFactory(factory: AllocatorFactory);

/// <summary> create a new allocator instance of a type. </summary>
declare function createAllocator(allocatorType: string): Allocator;

```

#### CPP interfaces

`Header`
```cpp
namespace napa {
    namespace memory {
        /// <summary> Interface for allocator factory. </summary>
        class AllocatorFactory {
            // ...
            public:
                Allocator* create(const char* allocatorType) = 0;
        };
        
        /// <summary> Concrete allocator class. 
        /// which will bind to different implementations.
        /// </summary>
        class Allocator {
            public:
                Allocator(AllocatorImpl* impl);

                void* allocate(size_t);

                void deallocate(void*);
        };

        /// <summary> Stl-style allocator adaptor from allocator. </summary>
        template <typename T>
        class StlAllocator {
            public:
                StlAllocator(Allocator* allocator);

                T* allocate(std::size_t n, const void *hint);

                void deallocate(T* pointer, std::size_t n);
        };
    }
}

```
`Implementation`
```cpp

/// <summary> This is defined in CPP. </summary>
struct AllocatorImpl {
    typedef void* (*AllocateFunction)(size_t);
    typedef void (*DeallocateFunction)(void*);
    typedef void (*DestructorFunction)(void*);

    void* allocatorHandle;

    AllocateFunction allocate;
    DeallocateFunction deallocate;
    DestructorFunction destruct;
};

/// <summary> Allocator factory that loads allocator from Dll. </summary>
class DllAllocatorFactory: public AllocatorFactory {
    // ...
    public:
        DllAllocatorFactory(const char* dllFileName);

        Allocator* create(const char* allocatorType);
};

```
`Dll exported functions`
```cpp

/// <summary> Create an allocator instance. </summary>
void* CreateAllocator(const char* allocatorType);

/// <summary> Allocate memory from an allocator. </summary>
void* Allocate(void* allocator, size_t size);

/// <summary> Free memory from an allocator. </summary>
void Deallocate(void* allocator, HANDLE memory);

/// <summary> Release allocator. </summary>
void ReleaseAllocator(void* allocator);
```

#### Built-in allocators
- StdAllocator

MS internal implementations
- Arena
- ThreadLocalAllocator

#### Stl containers with allocator

```cpp
namespace nape {
    template <typename T>
    using Vector = std::vector<T, napa::memory::StlAllocator<T>>;

    template <typename Key, typename T, typename Compare = std::less<Key> >
    using Map = std::map<Key, T, Compare, napa::memory::StlAllocator<std::pair<const Key, T>>>;

    template <typename Key, typename T, typename Compare = std::less<Key> >
    using UnorderedMap = std::unordered_map<Key, T, Compare, napa::memory::StlAllocator<std::pair<const Key, T>>>;

    /// More STL containers ...
}

```

*End of Document*