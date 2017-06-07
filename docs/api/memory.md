# namespace `memory`

## Table of Contents
- type [`Handle`](#handle)
- class [`Buffer`](#buffer)
- interface [`Shareable`](#shareable)
- interface [`Allocator`](#allocator)
    - [`allocator.allocate(size: number): Handle`](#allocator-allocate)
    - [`allocator.deallocate(handle: Handle, sizeHint: number): void`](#allocator-deallocate)
    - [`allocator.type: string`](#allocator-type)
- interface [`AllocatorDebugger`](#allocatorDebugger)
    - [`allocatorDebugger.debugInfo: string`](#allocatorDebugger-debugInfo)
- [`debugAllocator(allocator: Allocator): AllocatorDebugger`]
- object [`crtAllocator`](#crtAllocator)
- object [`defaultAllocator`](#defaultAllocator)
- [Memory allocation in C++ addon](#memory-allocation)

## APIs
## <a name="handle"></a>Type `Handle`
Handle is defined in TypeScript as below:
```ts
type Handle = [number, number]
``` 
It is a standard way to represent a 64-bit pointer in Napa.

## <a name="buffer"></a>Class `Buffer`
TBD
## <a name="allocator"></a>Interface `Allocator`
Interface for memory allocator that allocates memory for native objects. 

### <a name="allocator-allocate"></a>allocator.allocate(size: number): Handle
It allocates memory of requested size.

```ts
let handle = allocator.allocate(10);
```
### <a name="allocator-deallocate"></a>allocator.deallocate(handle: Handle, sizeHint: number): void
It deallocates memory from a input handle, with a size hint which is helpful for some C++ allocator implementations for deallocating memory.
```ts
allocator.deallocate(handle, 10);
```
### <a name="allocator-type"></a>allocator.type: string
It gets a string type identifier for the allocator, which will be useful during debugging purpose.


## <a name="allocatorDebugger"></a>Interface `AllocatorDebugger`
`AllocatorDebugger` extends interface `Allocator`, with a readonly property `debugInfo` to expose debug information. Basically an allocator debugger will use a pass-in allocator for memory allocation, meanwhile intercepting it to keep track of allocation count and size. 

### <a name="allocatorDebugger-debugInfo"></a>allocatorDebugger.debugInfo: string
It gets the debug information for allocation.

## <a name="debugAllocator"></a>debugAllocator(allocator: Allocator): AllocatorDebugger
It returns a simple allocator debugger, which returns debug information like below:
```json
{
    "allocate": 10,
    "allocateSize": 1024,
    "deallocate": 8,
    "deallocateSize": 912
}
```
## <a name="crtAllocator"></a>Object `crtAllocator`
It returns a C-runtime allocator from napa.dll. Its corresponding C++ part is `napa::memory::GetCrtAllocator()`.

## <a name="defaultAllocator"></a>Object `defaultAllocator`
It returns the default allocator from napa.dll. Its corresponding C++ part is `napa::memory::GetDefaultAllocator()`. Users can set default allocation/deallocation callback in `napa_allocator_set` API.

## <a name="memory-allocation"></a>Memory allocation in C++ addon
Memory allocation in C++ addon is tricky. A common pitfall is to allocate memory in one dll, but deallocate in another. This can cause issue if C-runtime in these 2 dlls are not compiled the same way. 

There are also advanced scenarios that user want to customize memory allocation. Napa provides APIs for customizing memory allocator as well.

### Recommended way of allocate memory.

### Customize memory allocation


