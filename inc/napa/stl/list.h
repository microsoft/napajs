// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/stl/allocator.h>
#include <list>

namespace napa {
    namespace stl {
        template <typename T>
        using List = std::list<T, napa::stl::Allocator<T>>;
    }
}

