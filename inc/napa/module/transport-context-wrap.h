// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/module.h>
#include <napa/transport/transport-context.h>

namespace napa {
namespace module {
    /// <summary> Interface for TransportContextWrap. </summary>
    class TransportContextWrap : public NAPA_OBJECTWRAP {
    public:
        /// <summary> Get transport context. </summary>
        virtual napa::transport::TransportContext* Get() = 0;

        virtual ~TransportContextWrap() = default;
    };
} // namespace module
} // namespace napa
