#pragma once

#include <napa/v8-helpers/maybe.h>
#include <napa/v8-helpers/function.h>
#include <napa/v8-helpers/flow.h>

namespace napa {
namespace v8_helpers {
    namespace JSON {
        /// <summary> JSON.stringify </summary>
        /// TODO @asib: Use v8::JSON::Stringify when available
        inline v8::MaybeLocal<v8::String> Stringify(v8::Isolate* isolate, const v8::Local<v8::Value>& value) {
            auto context = isolate->GetCurrentContext();
            v8::EscapableHandleScope scope(isolate);

            auto json = context->Global()
                ->Get(context, v8::String::NewFromUtf8(isolate, "JSON"))
                .ToLocalChecked()->ToObject(context)
                .ToLocalChecked();

            constexpr int argc = 1;
            v8::Local<v8::Value> argv[] = { value };
            return scope.Escape(LocalCast<v8::String>(Call(json, "stringify", argc, argv)));
        }

        /// <summary> JSON.parse </summary>
        inline v8::MaybeLocal<v8::Value> Parse(const v8::Local<v8::String>& jsonString) {
            return v8::JSON::Parse(jsonString);
        }
    }
}
}