// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <utility>

#include <v8.h>

namespace napa {
namespace v8_helpers {
    /// <summary> Uint32ptr size of multiple 32-bits. </summary>
    static const uint32_t UINTPTR_SIZE_IN_UINT32 = static_cast<uint32_t>(sizeof(uintptr_t) / sizeof(uint32_t));

    /// <summary> Convert a pointer value to V8 Uint32 array. </summary>
    /// <param name="isolate"> V8 Isolate instance. </summary>
    /// <param name="source"> Pointer value. </summary>
    /// <returns> V8 uint32 array </returns>
    inline v8::Local<v8::Array> UintptrToV8Uint32Array(v8::Isolate* isolate, uintptr_t source) {
        v8::EscapableHandleScope scope(isolate);

        auto context = isolate->GetCurrentContext();
        auto target = v8::Array::New(isolate, UINTPTR_SIZE_IN_UINT32);
        for (uint32_t i = 0; i < UINTPTR_SIZE_IN_UINT32; ++i) {
            auto value = static_cast<uint32_t>(source);
            target->CreateDataProperty(context, i, v8::Integer::NewFromUnsigned(isolate, value));
            source >>= 32;
        }
        return scope.Escape(target);
    }

    /// <summary> Convert a void pointer to V8 Uint32 array. </summary>
    /// <param name="isolate"> V8 Isolate instance. </summary>
    /// <param name="source"> Void pointer. </summary>
    /// <returns> V8 uint32 array </returns>
    inline v8::Local<v8::Array> PtrToV8Uint32Array(v8::Isolate* isolate, const void* pointer) {
        return UintptrToV8Uint32Array(isolate, reinterpret_cast<uintptr_t>(pointer));
    }

    /// <summary> Convert a V8 value (should be uint32 array) to uintptr. </summary>
    /// <param name="isolate"> V8 Isolate instance. </summary>
    /// <param name="source"> V8 uint32 array holding pointer value. </summary>
    /// <returns> The pair of converted pointer value and success/failure. </returns>
    inline std::pair<uintptr_t, bool> V8ValueToUintptr(v8::Isolate* isolate, const v8::Local<v8::Value>& source) {
        if (!source->IsArray()) {
            return std::make_pair(0, false);
        }

        auto numberArray = v8::Local<v8::Array>::Cast(source);
        if (numberArray->Length() != UINTPTR_SIZE_IN_UINT32) {
            return std::make_pair(0, false);
        }

        auto context = isolate->GetCurrentContext();
        uintptr_t result = 0;
        for (uint32_t i = 0; i < numberArray->Length(); ++i) {
            auto value = numberArray->Get(context, i).ToLocalChecked();
            if (!value->IsUint32()) {
                return std::make_pair(0, false);
            }
            result |= static_cast<uintptr_t>(value->Uint32Value()) << 32 * i;
        }
        return std::make_pair(result, true);
    }

    /// <summary> Convert a V8 value (should be uint32 array) to void pointer. </summary>
    /// <param name="isolate"> V8 Isolate instance. </summary>
    /// <param name="source"> V8 uint32 array holding pointer value. </summary>
    /// <returns> The pair of success and converted void pointer. </returns>
    template <typename T = void>
    inline std::pair<T*, bool> V8ValueToPtr(v8::Isolate* isolate, const v8::Local<v8::Value>& source) {
        auto result = V8ValueToUintptr(isolate, source);
        return std::make_pair(static_cast<T*>(reinterpret_cast<void*>(result.first)), result.second);
    }
} // namespace v8_helpers
} // namespace napa