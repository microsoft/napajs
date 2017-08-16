// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

let binding = require('./binding');

export interface LogFunction {
    (message: string): void;
    (section: string, message: string): void;
    (section: string, traceId: string, message: string): void;
}

export interface Log extends LogFunction {
    err(message: string): void;
    err(section: string, message: string): void;
    err(section: string, traceId: string, message: string): void;

    warn(message: string): void;
    warn(section: string, message: string): void;
    warn(section: string, traceId: string, message: string): void;

    info(message: string): void;
    info(section: string, message: string): void;
    info(section: string, traceId: string, message: string): void;

    debug(message: string): void;
    debug(section: string, message: string): void;
    debug(section: string, traceId: string, message: string): void;
}

export let log: Log = createLogObject();

enum LogLevel {
    Error = 0,
    Warning = 1,
    Info = 2,
    Debug = 3,
}

function dispatchLog(level: LogLevel, arg1: string, arg2?: string, arg3?: string) {
    if (arg3 != undefined) {
        binding.log(level, arg1, arg2, arg3);
    } else if (arg2 != undefined) {
        binding.log(level, arg1, undefined, arg2);
    } else {
        binding.log(level, undefined, undefined, arg1);
    }
}

function createLogObject(): Log {
    // napa.log()
    let logObj: any = function(
                          arg1: string, arg2?: string, arg3?: string) { dispatchLog(LogLevel.Info, arg1, arg2, arg3); }

                      // napa.log.err()
                      logObj.err = function(arg1: string,
                                            arg2?: string,
                                            arg3?: string) { dispatchLog(LogLevel.Error, arg1, arg2, arg3); }

                                   // napa.log.warn()
                                   logObj.warn =
        function(arg1: string, arg2?: string, arg3?: string) { dispatchLog(LogLevel.Warning, arg1, arg2, arg3); }

        // napa.log.info()
        logObj.info = function(
                          arg1: string, arg2?: string, arg3?: string) { dispatchLog(LogLevel.Info, arg1, arg2, arg3); }

                      // napa.log.debug()
                      logObj.debug = function(
            arg1: string, arg2?: string, arg3?: string) { dispatchLog(LogLevel.Debug, arg1, arg2, arg3); }

    return logObj;
}