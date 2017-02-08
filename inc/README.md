# napa/vanilla/inc: Napa public header files

## **Folder structure**
```
inc
  napa.h                  - C++ wrapper around C APIs.   
  napa-c.h                - C APIs for Napa.
  napa-app.h              - C++ API for Napa app engine.
  napa-module.h           - APIs for building C++ modules in Napa.
  napa/                   - Folder for internal headers.
    providers/            - Folder for providers interfaces.
        logging.h         - Interface for logging provider.
        metrics.h         - Interface for metric provider.
    ...
```
*Note: Users include "napa.h" and "napa_app.h" will static link with napa.lib, with is a thin adapter to access napa.dll.

## **C++ APIs**

```
napa                              : Namespace for Napa APIs.
  class Container                 : A pool of V8 isolate that share the same V8 parameters. 

napa::app                         : Namespace for Napa app engine APIs.
  class Engine                    : Interface to send request to Napa app layer.

napa::module                      : Namespace for Napa module APIs
  TBD

```