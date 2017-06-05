let binding = require('../binding');

// This variable is either defined by napa runtime, or not defined (hence node runtime)
declare var __in_napa: boolean;

/// <summary> 
///     Describes the available platform settings.
/// </summary>
export interface PlatformSettings {

    /// <summary> The logging provider to use when outputting logs. </summary>
    loggingProvider?: string;

    /// <summary> The metric provider to use when creating/setting metric values. </summary>
    metricProvider?: string;
}

/// <summary> Initialization of napa is only needed if we run in node. </summary>
let _initializationNeeded: boolean = (typeof __in_napa === 'undefined');

/// <summary> Empty platform settings. </summary>
let _platformSettings: PlatformSettings = {};

/// <summary> Sets the platform settings. Must be called fron node before the first container is created. </summary>
export function setPlatformSettings(settings: PlatformSettings) {
    if (!_initializationNeeded) {
        // If we don't need to initialize we can't set platform settings.
        throw new Error("Cannot set platform settings after napa was already initialized");
    }

    _platformSettings = settings;
    initialize();
}

export function initialize() {
    if (_initializationNeeded) {
        // Guard initializtion, should only be called once.
        binding.initialize(_platformSettings);
        _initializationNeeded = false;
    }
}
