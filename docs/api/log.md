# function `log`

## Table of Contents
- [Logging basics](#logging-basics)
- [C++ API](#c-api)
- [JavaScript API](#javascript-api)
    - [`log(message: string): void`](#log-message-string-void)
    - [`log(section: string, message: string): void`](#log-section-string-message-string-void)
    - [`log(section: string, traceId: string, message: string): void`](#log-section-string-traceid-string-message-string-void)
    - [`log.err(...)`](#log-error)
    - [`log.warn(...)`](#log-warn)
    - [`log.info(...)`](#log-info)
    - [`log.debug(...)`](#log-debug)
- [Using custom logging providers](#using-custom-logging-providers)
- [Developing custom logging providers](#developing-custom-logging-providers)

## Logging basics
Logging is a basic requirement for building services. `napajs` logging API enables developers to integrate their own logging capabilities in both JavaScript and C++ (addon) world.

A log row may contain following information:
- (Optional) Section: Useful as filter to treat different rows of logs. The treatment is defined by logging providers.
- (Optional) Trace ID: Useful field to join logs in the same transaction or request.
- (Required) Message: Log message.
- (Required) Logging level: 
    - Error: for application error.
    - Warn: for warining information.
    - Info: usually used for notifying service state change 
    - Debug: for debugging purpose

## C++ API
Include header: `<napa-log.h>`

Macros:
- LOG_ERROR(section, format, ...)
- LOG_ERROR_WITH_TRACEID(section, traceId, format, ...)
- LOG_WARNING(section, format, ...)
- LOG_WARNING_WITH_TRACEID(section, traceId, format, ...)
- LOG_INFO(section, format, ...)
- LOG_INFO_WITH_TRACEID(section, traceId, format, ...)
- LOG_DEBUG(section, format, ...)
- LOG_DEBUG_WITH_TRACEID(section, traceId, format, ...)

```cpp
#include <napa-log.h>

void MyFunction() {
    // ...
    LOG_ERROR("init", "error: %s", errorMessage.c_str());
}
```

## JavaScript API

### log(message: string): void
It logs a message. Using info level. 
*Functions `log` are shortcuts for `log.info`.

Example:
```ts
import * as napa from 'napajs';
napa.log('program started');
```

### log(section: string, message: string): void
It logs a message with a section. Using info level. 
Example:
```ts
napa.log('init', 'program started');
```

### log(section: string, traceId: string, message: string): void
It logs a message with a section, associating it with a traceId. Using info level.

Example:
```ts
napa.log('request', 'A1B2C3D4', 'request received');
```
### log.err(...)
It logs an error message. Three combinations of arguments are the same with `log`.

### log.warn(...)
It logs a warning message. Three combinations of arguments are the same with `log`.

### log.info(...)
It logs an info message. Three combinations of arguments are the same with `log`.

### log.debug(...)
It logs a debug message. Three combinations of arguments are the same with `log`.

## Using custom logging providers
Developers can hook up custom logging provider by calling the following before creation of any zones:
```ts
napa.runtime.setPlatformSettings({
    "loggingProvider": "<custom-logging-provider-module-name>"
}
```
## Developing custom logging providers
TBD