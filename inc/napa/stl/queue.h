#pragma once

#include <napa/stl/allocator.h>
#include <queue>

namespace napa {
    namespace stl {
        template <typename T>
        using Queue = std::queue<T, std::deque<T, napa::stl::Allocator<T>>>;

        template <typename T>
        using PriorityQueue = std::priority_queue<T, std::vector<T, napa::stl::Allocator<T>>>;
    }
}
