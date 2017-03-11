/// <summary>
///     Defines the possible log levels. 
///     This enum must be in sync with the native enum at napa::providers::LoggingProvider::Verboseness.
/// </summary>
export declare enum LogLevel {
    Error = 0,
    Warning = 1,
    Info = 2,
    Debug = 3,
}

/// </summary> The logger wrapper interface. </summary>
export interface LoggingProvider {
    log(level: LogLevel, section: string, traceId: string, message: string): void;
}

/// </summary> Set the underlying logging provider that log will dispatch to. </summary>
export declare function setProvider(provider: LoggingProvider): void;

export declare function err(message: string): void;
export declare function err(section: string, message: string): void;
export declare function err(section: string, traceId: string, message: string): void;
export declare function warn(message: string): void;
export declare function warn(section: string, message: string): void;
export declare function warn(section: string, traceId: string, message: string): void;
export declare function info(message: string): void;
export declare function info(section: string, message: string): void;
export declare function info(section: string, traceId: string, message: string): void;
export declare function debug(message: string): void;
export declare function debug(section: string, message: string): void;
export declare function debug(section: string, traceId: string, message: string): void;
