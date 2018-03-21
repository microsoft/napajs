// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

import * as zone from './zone/zone';
import * as impl from './zone/zone-impl';

import * as platform from './runtime/platform';

let binding = require('./binding');

/// <summary> Creates a new zone. </summary>
/// <summary> A unique id to identify the zone. </summary>
/// <param name="settings"> The settings of the new zone. </param>
export function create(id: string, settings: zone.ZoneSettings = zone.DEFAULT_SETTINGS) : zone.Zone {
    platform.initialize();
    return new impl.ZoneImpl(binding.createZone(id, settings));
}

/// <summary> Returns the zone associated with the provided id. </summary>
export function get(id: string) : zone.Zone {
    platform.initialize();
    return new impl.ZoneImpl(binding.getZone(id));
}

/// TODO: add function getOrCreate(id: string, settings: zone.ZoneSettings): Zone.

/// <summary> Define a getter property 'current' to retrieve the current zone. </summary>
export declare let current: zone.Zone;
Object.defineProperty(exports, "current", {
    get: function () : zone.Zone {
        platform.initialize();
        return new impl.ZoneImpl(binding.getCurrentZone());
    }
});

/// <summary> Define a getter property 'node' to retrieve node zone. </summary>
export declare let node: zone.Zone;
Object.defineProperty(exports, "node", {
    get: function () : zone.Zone {
        platform.initialize();
        return new impl.ZoneImpl(binding.getZone('node'));
    }
});

export * from './zone/zone';