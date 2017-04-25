#pragma once

#include <napa/stl/allocator.h>
#include <vector>

namespace napa {
    namespace stl {
        template <typename T>
        using Vector = std::vector<T, napa::stl::Allocator<T>>;
    }
}
