# Function `log`

## Table of Contents
- [Introduction](#intro)
- [C++ API](#cpp-api)
- [JavaScript API](#js-api)
    - [`log(message: string): void`](#log)
    - [`log(section: string, message: string): void`](#log-with-section)
    - [`log(section: string, traceId: string, message: string): void`](#log-with-traceid)
    - [`log.err(...)`](#log-err)
    - [`log.warn(...)`](#log-warn)
    - [`log.info(...)`](#log-info)
    - [`log.debug(...)`](#log-debug)
- [Using custom logging providers](#use-custom-providers)
- [Developing custom logging providers](#develop-custom-providers)

## <a name="intro"></a> Introduction
Logging is a basic requirement for building services. `napajs` logging API enables developers to integrate their own logging capabilities in both JavaScript and C++ (addon) world.

A log row may contain following information:
- (Optional) Section: Useful field to filter log rows. Treatment is defined by logging providers.
- (Optional) Trace ID: Useful field to join logs in the same transaction or request.
- (Required) Message: Log message.
- (Required) Logging level: 
    - Error: for application error.
    - Warn: for warining information.
    - Info: for notification. 
    - Debug: for debugging purpose.

## <a name="cpp-api"></a> C++ API
Include header: `<napa.h>`

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
#include <napa.h>

void MyFunction() {
    // ...
    LOG_ERROR("init", "error: %s", errorMessage.c_str());
}
```

## <a name="js-api"></a> JavaScript API

### <a name="log"></a> log(message: string): void
It logs a message. Using info level. 

*`log` is a shortcut for `log.info`.*

Example:
```js
var napa = require('napajs');
napa.log('program started');
```

### <a name="log-with-section"></a> log(section: string, message: string): void
It logs a message with a section. Using info level. 

Example:
```js
napa.log('init', 'program started');
```

### <a name="log-with-traceid"></a> log(section: string, traceId: string, message: string): void
It logs a message with a section, associating it with a traceId. Using info level.

Example:
```js
napa.log('request', 'A1B2C3D4', 'request received');
```
### <a name="log-err"></a> log.err(...)
It logs an error message. Three variation of arguments are the same with `log`.

### <a name="log-warn"></a> log.warn(...)
It logs a warning message. Three variation of arguments are the same with `log`.

### <a name="log-info"></a>log.info(...)
It logs an info message. Three variation of arguments are the same with `log`.

### <a name="log-debug"></a> log.debug(...)
It logs a debug message. Three combinations of arguments are the same with `log`.

## <a name="use-custom-providers"></a> Using custom logging providers
Developers can hook up custom logging provider by calling the following before creation of any zones:
```js
napa.runtime.setPlatformSettings({
    "loggingProvider": "<custom-logging-provider-module-name>"
}
```
## <a name="develop-custom-providers"></a> Developing custom logging providers
TBD
