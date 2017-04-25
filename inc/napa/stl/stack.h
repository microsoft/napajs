#pragma once

#include <napa/stl/allocator.h>
#include <stack>

namespace napa {
    namespace stl {
        template <typename T>
        using Stack = std::priority_queue<T, std::deque<T, napa::stl::Allocator<T>>>;
    }
}
