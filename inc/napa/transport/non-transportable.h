#pragma once

#include <napa-module.h>
#include <napa/module/common.h>

namespace napa {
namespace transport {

    /// <summary> Helper for implementing NonTransportable. </summary>
    /// <remarks> Reference: napajs/lib/transport/non-transportable.ts#@nontransportable </remarks>
    struct NonTransportable {
        /// <summary> It makes a wrap object extends NonTransportable.</summary>
        /// <param name='constructorTemplate'> Constructor function template of the wrap. </param>
        static void InitConstructor(v8::Local<v8::Function> constructor) {
            auto isolate = v8::Isolate::GetCurrent();
            v8::HandleScope scope(isolate);
            
            // Add static property '_cid' to class.
            constructor->Set(
                v8_helpers::MakeV8String(isolate, "_cid"), 
                v8_helpers::MakeV8String(isolate, "<non-transportable>"));
        }
    };
}
}