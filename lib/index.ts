// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

import { log } from './log';
import * as memory from './memory';
import * as metric from './metric';
import * as runtime from './runtime';
import * as store from './store';
import * as transport from './transport';
import * as zone from './zone';

export { log, memory, metric, runtime, store, transport, zone };

// Add execute proxy to global context.
import { call } from './zone/function-call';
(<any>(global))["__napa_zone_call__"] = call;