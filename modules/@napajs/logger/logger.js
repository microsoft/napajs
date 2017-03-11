"use strict";

/// <summary>
///     Defines the possible log levels. 
///     This enum must be in sync with the native enum at napa::providers::LoggingProvider::Verboseness.
/// </summary>
var LogLevel;
(function (LogLevel) {
    LogLevel[LogLevel["Error"] = 0] = "Error";
    LogLevel[LogLevel["Warning"] = 1] = "Warning";
    LogLevel[LogLevel["Info"] = 2] = "Info";
    LogLevel[LogLevel["Debug"] = 3] = "Debug";
})(LogLevel = exports.LogLevel || (exports.LogLevel = {}));

/// <summary> Default log implementation. </summary>
var ConsoleLoggingProvider = (function () {
    function ConsoleLoggingProvider() {
    }
    ConsoleLoggingProvider.prototype.log = function (level, section, traceId, message) {
        var output = "[" + LogLevel[level] + "] ";
        if (section != null) {
            output += "[" + section + "] ";
        }
        if (traceId != null) {
            output += "[" + traceId + "] ";
        }
        output += message;

        console.log(output);
    };
    return ConsoleLoggingProvider;
}());

var _loggingProvider = new ConsoleLoggingProvider();

function setProvider(provider) {
    // TODO @asib: uncomment the assert
    // assert(provider, "the provider to set can't be null");
    _loggingProvider = provider;
}

exports.setProvider = setProvider;

/// <summary> Log dispatcher that correctly routes the log parameters. </summary>
function log(level, arg1, arg2, arg3) {
    if (arg3 != null) {
        _loggingProvider.log(level, arg1, arg2, arg3);
    }
    else if (arg2 != null) {
        _loggingProvider.log(level, arg1, null, arg2);
    }
    else {
        _loggingProvider.log(level, null, null, arg1);
    }
}

function err(section, traceId, message) {
    log(LogLevel.Error, section, traceId, message);
}
exports.err = err;

function warn(section, traceId, message) {
    log(LogLevel.Warning, section, traceId, message);
}
exports.warn = warn;

function info(section, traceId, message) {
    log(LogLevel.Info, section, traceId, message);
}
exports.info = info;

function debug(section, traceId, message) {
    log(LogLevel.Debug, section, traceId, message);
}
exports.debug = debug;
