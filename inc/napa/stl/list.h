#pragma once

#include <napa/stl/allocator.h>
#include <list>

namespace napa {
    namespace stl {
        template <typename T>
        using List = std::list<T, napa::stl::Allocator<T>>;
    }
}

