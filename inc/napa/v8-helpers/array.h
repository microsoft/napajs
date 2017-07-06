#pragma once

#include "conversion.h"

#include <v8.h>
#include <vector>

namespace napa {
namespace v8_helpers {
    /// <summary> Convert V8::Array to std::vector </summary>
    template <typename ValueType>
    inline std::vector<ValueType> V8ArrayToVector(v8::Isolate* isolate, const v8::Local<v8::Array>& array) {
        auto context = isolate->GetCurrentContext();

        std::vector<ValueType> res;
        res.reserve(array->Length());

        for (uint32_t i = 0; i < array->Length(); i++) {
            res.emplace_back(V8ValueTo<ValueType>(array->Get(context, i).ToLocalChecked()));
        }
        return res;
    }
}
}
