# Logger module

This module provides uniform APIs for logging in napa.

## Simple usage
This example shows the way a client should use the logger APIs.

```ts
import logger = require('@napajs/logger');

logger.err("An error message");
logger.err("SectionName", "An error message with section");
logger.err("SectionName", "123-456-789" /* trace id */, "An error message with section and trace id");

logger.warn("A warning message");
logger.warn("SectionName", "A warning message with section");
logger.warn("SectionName", "123-456-789" /* trace id */, "A warning message with section and trace id");

logger.info("An info message");
logger.info("SectionName", "An info message with section");
logger.info("SectionName", "123-456-789" /* trace id */, "An info message with section and trace id");

logger.debug("A debug message");
logger.debug("SectionName", "A debug message with section");
logger.debug("SectionName", "123-456-789" /* trace id */, "A debug message with section and trace id");
```

## Setup provider
This example shows how to setup a custom logging provider.

```ts
import logger = require('@napajs/logger');

class MyLoggingProvider implements logger.LoggingProvider {
    log(level: logger.LogLevel, section: string, traceId: string, message: string): void {
        console.log("[MyLoggingProvider] " + message);
    }
}

// If setProvider was not called the default provider outputs to console.
logger.setProvider(new MyLoggingProvider());
```

## Napa builtin provider
This example shows how to configure the napa built in provider.

```ts
import logger = require('@napajs/logger');

import napa = require('napajs');
logger.setProvider(napa.getLoggingProvider());
```
