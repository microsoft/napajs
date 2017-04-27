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
}
}