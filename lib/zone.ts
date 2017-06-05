import * as napa from './zone/napa-zone';
import * as node from './zone/node-zone';
import * as zone from './zone/zone';

import * as platform from './runtime/platform';

let binding = require('./binding');

// This variable is either defined by napa runtime, or not defined (hence node runtime)
declare var __in_napa: boolean;

/// <summary> Creates a new zone. </summary>
/// <summary> A unique id to identify the zone. </summary>
/// <param name="settings"> The settings of the new zone. </param>
export function create(id: string, settings?: zone.ZoneSettings) : zone.Zone {
    platform.initialize();
    return new napa.NapaZone(binding.createZone(id, settings));
}

/// <summary> Returns the zone associated with the provided id. </summary>
export function get(id: string) : zone.Zone {
    platform.initialize();
    if (id === "node") {
        return new node.NodeZone();
    }

    return new napa.NapaZone(binding.getZone(id));
}

/// <summary> Define a getter property 'current' to retrieves the current zone. </summary>
export declare let current: zone.Zone;
Object.defineProperty(exports, "current", {
    get: function () : zone.Zone {
        platform.initialize();
        if (typeof __in_napa !== 'undefined') {
            return new napa.NapaZone(binding.getCurrentZone());
        }

        return new node.NodeZone();
    }
});

export * from './zone/zone';