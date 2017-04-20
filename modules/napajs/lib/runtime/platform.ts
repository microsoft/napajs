import * as zone from './zone';
import * as napa from './napa-zone';
import * as node from './node-zone';
var addon = require('../../bin/addon');

/// <summary> 
///     Describes the available platform settings, these setting include cross zone settings
///     as well as zone specific settings. The zone specific settings are used as defaults
///     when a zone is created without specifying them.
/// </summary>
export interface PlatformSettings extends zone.ZoneSettings {

    /// <summary> The logging provider to use when outputting logs. </summary>
    loggingProvider?: string;

    /// <summary> The metric provider to use when creating/setting metric values. </summary>
    metricProvider?: string;
}

declare var __in_napa: boolean;

/// <summary> Initialization of napa is only needed if we run in node. </summary>
let _initializationNeeded: boolean = (typeof __in_napa === undefined);

/// <summary> Empty platform settings. </summary>
let _platformSettings: PlatformSettings = {};

/// <summary> Returns the logging provider. </summary>
export function getLoggingProvider() {
    return addon.getLoggingProvider();
}

/// <summary> Returns the metric provider. </summary>
export function getMetricProvider() {
    return addon.getMetricProvider();
}

/// <summary> Sets the platform settings. Must be called fron node before the first container is created. </summary>
export function setPlatformSettings(settings: PlatformSettings) {
    if (!_initializationNeeded) {
        // If we don't need to initialize we can't set platform settings.
        throw new Error("Cannot set platform settings after napa was already initialized");
    }

    _platformSettings = settings;
}

/// <summary> Creates a new zone. </summary>
/// <summary> A unique id to identify the zone. </summary>
/// <param name="settings"> The settings of the new zone. </param>
export function createZone(id: string, settings?: zone.ZoneSettings) : zone.Zone {
    if (_initializationNeeded) {
        // Lazy initialization of napa when first zone is created.
        addon.initialize(_platformSettings);
        _initializationNeeded = false;
    }

    return new napa.NapaZone(addon.createZone(id, settings));
}

/// <summary> Returns the zone associated with the provided id. </summary>
export function getZone(id: string) : zone.Zone {
    if (id === "node") {
        return new node.NodeZone();
    }

    return new napa.NapaZone(addon.getZone(id));
}

/// <summary> Returns the current zone. </summary>
export let currentZone: zone.Zone = getCurrentZone();

function getCurrentZone() : zone.Zone {
    if (typeof __in_napa !== undefined) {
        return new napa.NapaZone(addon.getCurrentZone());
    }
    
    return new node.NodeZone();
}
