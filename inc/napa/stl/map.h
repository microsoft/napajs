// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/stl/allocator.h>
#include <map>

namespace napa {
    namespace stl {
        template <typename Key, typename T, typename Compare = std::less<Key>>
        using Map = std::map<Key, T, Compare, napa::stl::Allocator<std::pair<const Key, T>>>;
        
        template <typename Key, typename T, typename Compare = std::less<Key>>
        using MultiMap = std::multimap<Key, T, Compare, napa::stl::Allocator<std::pair<const Key, T>>>;
    }
}
