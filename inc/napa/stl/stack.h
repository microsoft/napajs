// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/stl/allocator.h>

#include <stack>

namespace napa {
namespace stl {
    template <typename T>
    using Stack = std::priority_queue<T, std::deque<T, napa::stl::Allocator<T>>>;
}
} // namespace napa
