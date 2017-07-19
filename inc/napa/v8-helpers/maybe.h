// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <v8.h>

namespace napa {
namespace v8_helpers {
    /// <summary> Cast MaybeLocal from source type to target type. </summary>
    template <typename T, typename S>
    v8::MaybeLocal<T> MaybeCast(v8::MaybeLocal<S> handle) {
        if (handle.IsEmpty()) {
            return v8::MaybeLocal<T>();
        }
        return v8::Local<T>::Cast(handle.ToLocalChecked());
    }

    /// <summary> Cast MaybeLocal to Local from source type to target type with default value. </summary>
    template <typename T, typename S>
    v8::Local<T> ToLocal(v8::MaybeLocal<S> handle, v8::Local<T> defaultValue = v8::Local<T>()) {
        if (handle.IsEmpty()) {
            return defaultValue;
        }
        return v8::Local<T>::Cast(handle.ToLocalChecked());
    }
}
}