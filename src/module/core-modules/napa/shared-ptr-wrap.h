// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/module.h>
#include <napa/module/shareable-wrap.h>

namespace napa {
namespace module {

    /// <summary> It wraps C++ std::shared_ptr<T> and allow it be shared across isolates. </summary>
    /// <remarks> see napajs/lib/memory/shared-ptr-wrap.d.ts#SharedPtrWrap </remarks>
    class SharedPtrWrap : public ShareableWrap {
    public:
        /// <summary> Init SharedPtrWrap. </summary>
        static void Init();

        /// <summary> Declare constructor in public, so we can export class constructor in JavaScript world. </summary>
        static v8::Persistent<v8::Function> _constructor;

        /// <summary> Exported class name. </summary>
        static constexpr const char* exportName = "SharedPtrWrap";
    };
}
}