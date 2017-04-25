#pragma once

#include <napa/stl/allocator.h>
#include <deque>

namespace napa {
    namespace stl {
        template <typename T>
        using Deque = std::deque<T, napa::stl::Allocator<T>>;
    }
}
