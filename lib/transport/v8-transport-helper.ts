// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

import { Shareable } from '../memory/shareable';

/// <summary>
/// SharedPtrWrap of SerializedData.
/// </summary>
export interface SerializedData extends Shareable {
}

export function serializeValue(jsValue: any): SerializedData {
    return require('../binding').v8SerializeValue(jsValue);
}

export function deserializeValue(serializedData: SerializedData): any {
    return require('../binding').v8DeserializeValue(serializedData);
}