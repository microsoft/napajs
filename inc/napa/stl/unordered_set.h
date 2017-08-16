// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/stl/allocator.h>
#include <unordered_set>

namespace napa {
namespace stl {
    template <typename Key, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>, >
    using UnorderedSet = std::unordered_set<Key, Hash, KeyEqual, napa::stl::Allocator<Key>>;

    template <typename Key, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>, >
    using UnorderedMultiSet = std::unordered_multiset<Key, Hash, KeyEqual, napa::stl::Allocator<Key>>;
}
}
