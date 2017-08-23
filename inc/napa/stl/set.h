// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/stl/allocator.h>

#include <set>

namespace napa {
namespace stl {
    template <typename Key, typename Compare = std::less<Key>>
    using Set = std::set<Key, Compare, napa::stl::Allocator<Key>>;

    template <typename Key, typename Compare = std::less<Key>>
    using MultiSet = std::multiset<Key, Compare, napa::stl::Allocator<Key>>;
} // namespace stl
} // namespace napa
