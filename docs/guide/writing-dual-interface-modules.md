# Best practices writing dual-interface modules.

Dual-interface modules are Napa modules that expose both JavaScript and C++ interfaces. The major purpose of introducing a dual-interface module can be one of the following, or both:
1) Sharing C++ objects between host process and Napa add-ons without extensive marshalling/unmarshalling.
2) Provide 2 implementations - Javascript for agility, and C++ for performance. Developers can quickly iterate ideas in JavaScript, once logic is finalized, we can use C++ interfaces to hook up pretty easily.

## Pitfalls in dual-interface modules.
### Memory allocation and deallocation across boundaries.
Dll might use different heap other than the process' heap depending on which C runtime libraries they are linked to. If they depends on a different C runtime, memory allocation/deallocation should always happen in the same compilation unit. See [this](http://stackoverflow.com/questions/10820114/do-statically-linked-dlls-use-a-different-heap-than-the-main-program) for details.

For modules to be usable for different host process, it's always a best practice to **always allocate/deallocate memory within a compilation boundary, either from Dll or host process.**
Sometimes memory allocation is implicit, like inserting an element to a container, or using an objet on heap. 

### Binary versioning
Another issue is the version conflict of linked binary. For example, if both the host process and your module Dll depends on the same C++ classes. You need to ensure they linked to the same version of the C++ class implementation, otherwise the memory layout might be totally different. 

Since multiple version of host processes can consume the same module, it's not possible to align the module's version with each host process. Instead, we should always let host services to compile against module's library versions. That is, distribute the C++ library together with your dual-interface module.

## Reference architecture

```
 +-------------------------------+                              +----------------------------------------+
 |  Host process code (host.cpp) |                              |       JavaScript code (program.js)     |
 |                               |   <call to JS code>          | +------ create JS object.              |
 | container.run('program.js') --|----------------------------->| |   +-------------------------------+  |
 |                               |                              | |   |  dual-interface module        |  |
 |                               |                              | |   |        (addon.napa)           |  |
 |                               |                              | |   |                               |  |
 |                               |                              | +---|------> create object   -------|--|-----+
 |                               |  <c++ object in raw pointer> |     |                               |  |     |
 |      <consume raw pointer> <--|------------------------------|-<<--|------- return raw pointer     |  |     |
 |              ...              |                              |     |                               |  |     |
 |       object.destroy()        |                              |     |                               |  |     |
 |          |                    |                              |     |                               |  |     |
 +-------------------------------+                              |     +-------------------------------+  |     |
            |                                                   +----------------------------------------+     |
            |                                                                                                  |
            |                                                                                                  |
            |     +----------------------------------------------------+                                       |
            |     |                Shared library Dll (shared.cpp)     |                                       |
            |     |                                                    |                                       |
            |     |          <allocate memory and construct> <-------------------------------------------------+
            |     |                                                    |
            +-----|--------> <destruct and free memory>                |
                  |                                                    |
                  +----------------------------------------------------+
```
Highlights:
1) Shared library that is referenced by both host process and Napa module should be separated as a standalone Dll. 
2) Headers, static lib and dll of the  shared library should always be distributed together with the module.
3) For any memory allocation and deallocation from shared library, always allocate/deallocate from the Dll, to avoid allocation/free from different compilation units.

Complex cases: When host process already static linked with a legacy library, and new Napa module needs to depend on the library. How do we deal with shared library version confliction?
- If host process and module functions independently, and the shared library doesn't iteract between compilation boundaries, then it is okay to treat them as not shared.
- If host process and module iteract with each other using code from the shared library, then we need to ensure module is using Dll with the same version as host process' version. A way may be manually replace the Dll in the module directory from the build of host service after NPM install.


### Suggested module layout
```
  <module-name>
    |---inc          : header files for C++ to compile against.
    |---lib          : JavaScript files for this module.
    |---bin          : Binaries for your module, including wrap (.napa and .node), static lib and dll for shared library that host service can link against.
    |---README.md    : Documentation for both JavaScript/TypeScript and C++ interfaces and usages.
    |---...

```


*End of Document*