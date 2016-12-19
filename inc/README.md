# napa/vanilla/inc: Napa public header files

## **Folder structure**
```
inc
  napa_runtime.h                  - Facade for C++ API of Napa runtime.   
  napa_runtime_c.h                - Facade for C API for Napa runtime.
  napa_app.h                      - Facade for C++ API for Napa app engine.
  napa_app_c.h                    - Facade for C API for Napa app engine.
  napa_module.h                   - Facade for building C++ modules in Napa.
  napa/                           - Folder for internal headers
    logging.h (TBD)
    metrics.h (TBD)
    ...
```
*Note: Users include "napa_runtime.h" and "napa_app.h" will static link with napa.lib, with is a thin adapter to access napa.dll.

## **C++ APIs**

```
napa::runtime                     : Namespace for Napa runtime APIs.
  class Container                 : A pool of V8 isolate that share the same V8 parameters. 

napa::app                         : Namespace for Napa app engine APIs.
  class Engine                    : Interface to send request to Napa app layer.

napa::module                      : Namespace for Napa module APIs
  TBD

```