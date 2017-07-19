// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/exports.h>

namespace napa {
namespace providers {

    /// <summary> Interface for a generic logging provider. </summary>
    /// <remarks> 
    ///     Ownership of this logging provider belongs to the shared library which created it. Hence the explicit
    ///     Destroy method in this class. To simplify memory management across multiple shared libraries, this class
    ///     can only be created via a factory method provided by the shared library. When it is no longer needed,
    ///     the caller may call Destroy() which will tell the shared library which created it to dispose of the object.
    /// </remarks>
    class LoggingProvider {
    public:

        /// <summary> Represents verboseness for logging. </summary>
        enum class Verboseness {
            Error = 0,
            Warning,
            Info,
            Debug
        };

        /// <summary> Logs a message. </summary>
        /// <param name="section"> Logging section. </param>
        /// <param name="level"> Logging verboseness level. </param>
        /// <param name="traceId"> Trace ID. </param>
        /// <param name="file"> The source file this log message originated from. </param>
        /// <param name="line"> The source line this log message origiated from. </param>
        /// <param name="message"> The message. </param>
        virtual void LogMessage(
            const char* section,
            Verboseness level,
            const char* traceId,
            const char* file,
            int line,
            const char* message) = 0;

        /// <summary> Returns if logging is enabled for section/level/title. </summary>
        /// <param name="section"> Logging section. </param>
        /// <param name="level"> Logging verboseness level. </param>
        /// <returns> True if logging will occur for section/level, false otherwise. </returns>
        virtual bool IsLogEnabled(const char* section, Verboseness level) = 0;

        /// <summary> Explicitly destroys the logging provider. </summary>
        virtual void Destroy() = 0;

    protected:

        /// <summary> Prevent calling delete on the interface. Must use Destroy! </summary>
        virtual ~LoggingProvider() = default;
    };

    /// <summary> Exports a getter function for retrieves the configured logging provider. </summary>
    NAPA_API LoggingProvider& GetLoggingProvider();

    /// <summary> Singnature  of the logging provider factory method. </summary>
    typedef LoggingProvider* (*CreateLoggingProvider)();
}
}
