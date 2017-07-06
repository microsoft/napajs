#pragma once

#include <napa/exports.h>

#include <cstddef>
#include <stdint.h>

namespace napa {
namespace providers {

    /// <summary> Enumeration of metric type. </summary>
    enum class MetricType {
        Number = 0,
        Rate,
        Percentile,
    };

    /// <summary> Interface to represents a multi-dimensional metric with a maximum dimensionality of 64. </summary>
    class Metric {
    public:

        /// <summary> Sets a metric value with varidic dimension arguments. </summary>
        /// <param name="value"> Int64 value. </param>
        /// <param name="numberOfDimensions"> Number of dimensions being set. </param>
        /// <param name="dimensionValues"> Array of dimension value names. </param>
        /// <returns> Success/Fail. </returns>
        /// <remarks>
        ///     The number of dimension values must exactly match the number of dimensions provided when
        ///     creating this metric.
        /// </remarks>
        virtual bool Set(int64_t value, size_t numberOfDimensions, const char* dimensionValues[]) = 0;

        /// <summary>
        ///     Increments a metric value with variadic dimension arguments.
        ///     Use mainly to simplify rate counters.
        /// </summary>
        /// <param name="value"> UInt64 value to increment. </param>
        /// <param name="numberOfDimensions"> Number of dimensions being set. </param>
        /// <param name="dimensionValues"> Array of dimension value names. </param>
        /// <returns> Success/Fail. </returns>
        /// <remarks>
        ///     The number of dimension values must exactly match the number of dimensions
        ///     provided when creating this metric.
        /// </remarks>
        virtual bool Increment(uint64_t value, size_t numberOfDimensions, const char* dimensionValues[]) = 0;

        /// <summary>
        ///     Decrements metric value with varidic dimension arguments.
        ///     Use mainly to simplify rate counters.
        /// </summary>
        /// <param name="value"> UInt64 value to decrement. </param>
        /// <param name="numberOfDimensions"> Number of dimensions being set. </param>
        /// <param name="dimensionValues"> Array of dimension value names. </param>
        /// <returns> Success/Fail. </returns>
        /// <remarks>
        ///     The number of dimension values must exactly match the number of dimensions
        ///     provided when creating this metric.
        /// </remarks>
        virtual bool Decrement(uint64_t value, size_t numberOfDimensions, const char* dimensionValues[]) = 0;

        /// <summary> Explicitly destroys the Metric. </summary>
        /// <remarks>
        ///     Consumers are not required to call this.
        ///     The MetricProvider owns this class and will automatically perform cleanup on shutdown.
        /// </remarks>
        virtual void Destroy() = 0;

    protected:

        ///<summary> Prevent calling delete on the interface. Must use Destroy! </summary>
        virtual ~Metric() = default;
    };


    /// <summary> Interface for a generic metric provider. </summary>
    /// <remarks> 
    ///     Ownership of this metric provider belongs to the shared library which created it. Hence the explicit
    ///     Destroy method in this class. To simplify memory management across multiple shared libraries, this class
    ///     can only be created via a factory method provided by the shared library. When it is no longer needed,
    ///     the caller may call Destroy() which will tell the shared library which created it to dispose of the object.
    /// </remarks>
    class MetricProvider {
    public:

        /// <summary>
        ///     Gets or creates a N-dimensional metric. Metric objects are owned and cached by this class.
        ///     Up to 64 dimensions may be used.</summary>
        /// <param name="section"> Section of the metric.</param>
        /// <param name="name"> Name of the metric.</param>
        /// <param name="type"> Type of the metric.</param>
        /// <param name="dimensions">
        ///     Number of dimensions requested for this metric.
        ///     Represents the size of the array passed in for p_dimensionNames.
        /// </param>
        /// <param name="dimensionNames"> Array of dimension names being requested for this metric.</param>
        /// <remarks>
        ///     The IMetric class returned is owned and cached by this class.
        ///     Callers are not required to call destroy() on the Metric.
        /// </remarks>
        virtual Metric* GetMetric(
            const char* section,
            const char* name,
            MetricType type,
            size_t dimensions,
            const char* dimensionNames[]) = 0;

        ///<summary> Explicitly destroys the metric provider. </summary>
        virtual void Destroy() = 0;

    protected:

        ///<summary> Prevent calling delete on the interface. Must use Destroy! </summary>
        virtual ~MetricProvider() = default;
    };

    /// <summary> Exports a getter function for retrieves the configured metric provider. </summary>
    NAPA_API MetricProvider& GetMetricProvider();

    typedef MetricProvider* (*CreateMetricProvider)();
}
}
