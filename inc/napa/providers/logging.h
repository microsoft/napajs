#pragma once

namespace napa {
namespace providers {

    /// <summary> Represents verboseness for logging. </summary>
    enum class Verboseness {
        Error,
        Warning,
        Info,
        Debug
    };

    /// <summary> Interface for a generic logging provider. </summary>
    /// <remarks> 
    ///     Ownership of this logging provider belongs to the shared library which created it. Hence the explicit
    ///     Destroy method in this class. To simplify memory management across multiple shared libraries, this class
    ///     can only be created via a factory method provided by the shared library. When it is no longer needed,
    ///     the caller may call Destroy() which will tell the shared library which created it to dispose of the object.
    /// </remarks>
    class LoggingProvider {
    public:

        /// <summary> Logs a formatted message. </summary>
        /// <param name="section"> Logging section. </param>
        /// <param name="level"> Logging verboseness level. </param>
        /// <param name="traceId"> Trace ID. </param>
        /// <param name="title"> Title. </param>
        /// <param name="format"> Formatted string with variadic arguments. </param>
        virtual void Log(
            const char* section,
            Verboseness level,
            const char* traceId,
            const char* title,
            const char* format, ...) = 0;

        /// <summary> Returns if logging is enabled for section/level/title. </summary>
        /// <param name="section"> Logging section. </param>
        /// <param name="level"> Logging verboseness level. </param>
        /// <param name="title"> Logging title. </param>
        /// <returns> True if logging will occur for section/level/title, false otherwise. </returns>
        virtual bool IsLogEnabled(const char* section, Verboseness level, const char* title) = 0;

        /// <summary> Explicitly destroys the logging provider. </summary>
        virtual void Destroy() = 0;

    protected:

        /// <summary> Prevent calling delete on the interface. Must use Destroy! </summary>
        virtual ~LoggingProvider() = default;
    };

    /// <summary> Singnature  of the logging provider factory method. </summary>
    typedef LoggingProvider* (*CreateLoggingProvider)();
}
}
