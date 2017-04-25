#pragma once

#include <napa/stl/allocator.h>
#include <string>

namespace napa {
    namespace stl {
        template <typename CharT, typename Traits = std::char_traits<CharT>>
        using BasicString = std::basic_string<CharT, Traits, napa::stl::Allocator<CharT>>;

        typedef BasicString<char> String;
    }
}
