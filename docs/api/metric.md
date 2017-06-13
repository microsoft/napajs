# namespace `metric`

## Table of Contents
- [Metric basics](#metric-basics)
- [C++ API](#c-api)
    - interface [`Metric`](#interface-metric)
    - interface [`MetricProvider`](#interface-metric-provider)
    - function [`MetricProvider& GetMetricProvider()`](#metricprovider-getmetricprovider)
- [JavaScript API](#javascript-api)(#javascript-api)
    - enum [`MetricType`](#metrictype)
    - class [`Metric`](#class-metric)
        - [`set(value: number, dimensions?: string[]): void`](#set-value-number-dimensions-string)
        - [`increment(dimensions?: string[]): void`]();
        - [`decrement(dimensions?: string[]): void`]();
    - function [`get(section: string, name: string, type: MetricType, dimensionNames: string[])`]()

- [Using custom metric providers](#using-custom-metric-providers)
- [Developing custom metric providers](#developing-custom-metric-providers)

## Metric basics
Similar as logging, metric a basic requirement for creating monitorable services. `napajs` metric API enables developers to use their own metric system in both JavaScript and C++ (addon) world.

A metric may contain following information:
- (Required) Section: The group or category of the metric.
- (Required) Name: Name of the metric. Section/Name combination should be unique in the system.
- (Required) Metric type: Type of the metric, which can be
    - Number: A absolute number, e.g: PrivateBytes.
    - Rate: A flowing volume in number, e.g: QueryPerSecond.
    - Percentile: A absolute number that needs to be sampled by percentiles, e.g: SuccessLatency.
- (Required) Dimensions: A metric can have multiple dimensions, each dimension can bind with a string value at runtime. e.g: IncomingRequestRate can have 2 dimensions: ['client-id', 'request-type'].

## C++ API
### Interface Metric
```cpp
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
```
### Interface MetricProvider
```cpp

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
```
### function `MetricProvider& GetMetricProvider()`
```cpp
/// <summary> Exports a getter function for retrieves the configured metric provider. </summary>
NAPA_API MetricProvider& GetMetricProvider();
```
## JavaScript API
```ts
export enum MetricType {
    Number = 0,
    Rate,
    Percentile,
}

export interface Metric {
    section: string;
    name: string;

    set(value: number, dimensions?: string[]): void;
    increment(dimensions?: string[]): void;
    decrement(dimensions?: string[]): void;
}

export function get(
    section: string, 
    name: string, 
    type: MetricType, 
    dimensions: string[] = []) : Metric;
```

Example:
```ts
import * as napa from 'napajs';
let metric = napa.metric.get('app1', 'counter1', napa.metric.MetricType.Number, []);
metric.increment([]);
```
## Using custom metric providers
Developers can hook up custom metric provider by calling the following before creation of any zones:
```ts
napa.runtime.setPlatformSettings({
    "metricProvider": "<custom-metric-provider-module-name>"
}
```
## Developing custom metric providers
TBD
