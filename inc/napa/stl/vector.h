// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/stl/allocator.h>

#include <vector>

namespace napa {
namespace stl {
    template <typename T>
    using Vector = std::vector<T, napa::stl::Allocator<T>>;
}
} // namespace napa
