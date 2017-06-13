# namespace `memory`

## Table of Contents
- type [`Handle`](#type-handle)
- class [`Buffer`](#class-buffer)
- interface [`Shareable`](#interface-shareable)
- interface [`Allocator`](#interface allocator)
    - [`allocator.allocate(size: number): Handle`](#allocator-allocate-size-number-handle)
    - [`allocator.deallocate(handle: Handle, sizeHint: number): void`](#allocator-deallocate-handle-handle-sizehint-number-void)
    - [`allocator.type: string`](#allocator-type-string)
- interface [`AllocatorDebugger`](#interface-allocatordebugger)
    - [`allocatorDebugger.debugInfo: string`](#allocatordebugger-debugInfo-string)
- function [`debugAllocator(allocator: Allocator): AllocatorDebugger`](#debugallocator-allocator-allocator-allocatordebugger)
- object [`crtAllocator`](#object-crtallocator)
- object [`defaultAllocator`](#object-defaultallocator)
- [Memory allocation in C++ addon](#memory-allocation-in-c-addon)

## APIs
## Type `Handle`
Handle is defined in TypeScript as below:
```ts
type Handle = [number, number]
``` 
It is a standard way to represent a 64-bit pointer in Napa.

## Class `Buffer`
TBD

## Interface `Shareable`
Interface for native object wrap that can be shared across multiple JavaScript threads.

## Interface `Allocator`
Interface for memory allocator that allocates memory for native objects. 

### allocator.allocate(size: number): Handle
It allocates memory of requested size.

```ts
let handle = allocator.allocate(10);
```
### allocator.deallocate(handle: Handle, sizeHint: number): void
It deallocates memory from a input handle, with a size hint which is helpful for some C++ allocator implementations for deallocating memory.
```ts
allocator.deallocate(handle, 10);
```
### allocator.type: string
It gets a string type identifier for the allocator, which will be useful during debugging purpose.

## Interface `AllocatorDebugger`
`AllocatorDebugger` extends interface `Allocator`, with a readonly property `debugInfo` to expose debug information. Basically an allocator debugger will use a pass-in allocator for memory allocation, meanwhile intercepting it to keep track of allocation count and size. 

### allocatorDebugger.debugInfo: string
It gets the debug information for allocation.

## debugAllocator(allocator: Allocator): AllocatorDebugger
It returns a simple allocator debugger, which returns debug information like below:
```json
{
    "allocate": 10,
    "allocateSize": 1024,
    "deallocate": 8,
    "deallocateSize": 912
}
```
## Object `crtAllocator`
It returns a C-runtime allocator from napa.dll. Its corresponding C++ part is `napa::memory::GetCrtAllocator()`.

## Object `defaultAllocator`
It returns the default allocator from napa.dll. Its corresponding C++ part is `napa::memory::GetDefaultAllocator()`. Users can set default allocation/deallocation callback in `napa_allocator_set` API.

## Memory allocation in C++ addon
Memory allocation in C++ addon is tricky. A common pitfall is to allocate memory in one dll, but deallocate in another. This can cause issue if C-runtime in these 2 dlls are not compiled the same way. 

There are also advanced scenarios that user want to customize memory allocation. Napa provides APIs for customizing memory allocator as well.

### Recommended way of allocate memory.
TBD

### Customize memory allocation
TBD

