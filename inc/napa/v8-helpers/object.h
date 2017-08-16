// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/v8-helpers/string.h>
#include <v8.h>
#include <unordered_map>

namespace napa {
namespace v8_helpers {
    /// <summary> Convert a V8 object to an map. </summary>
    template <typename ValueType>
    inline std::unordered_map<std::string, ValueType> V8ObjectToMap(v8::Isolate* isolate,
                                                                    const v8::Local<v8::Object>& obj) {

        auto context = isolate->GetCurrentContext();
        std::unordered_map<std::string, ValueType> res;

        auto maybeProps = obj->GetOwnPropertyNames(context);
        if (!maybeProps.IsEmpty()) {
            auto props = maybeProps.ToLocalChecked();
            res.reserve(props->Length());

            for (uint32_t i = 0; i < props->Length(); i++) {
                auto key = props->Get(context, i).ToLocalChecked();
                auto value = obj->Get(context, key).ToLocalChecked();

                v8::String::Utf8Value keyString(key->ToString(context).ToLocalChecked());
                res.emplace(*keyString, V8ValueTo<ValueType>(value));
            }
        }
        return res;
    }
}
}