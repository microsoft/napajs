// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/stl/allocator.h>
#include <string>

namespace napa {
    namespace stl {
        template <typename CharT, typename Traits = std::char_traits<CharT>>
        using BasicString = std::basic_string<CharT, Traits, napa::stl::Allocator<CharT>>;

        typedef BasicString<char> String;
        typedef BasicString<char16_t> U16String;
    }
}

#if defined(__GNUC__) && !defined(__clang__)

namespace std {
    // std::hash specialization for napa::stl::String.
    template<>
    struct hash<napa::stl::String> : public __hash_base<size_t, napa::stl::String> {
        size_t operator()(const napa::stl::String& s) const noexcept {
            return std::_Hash_impl::hash(s.data(), s.length() * sizeof(char));
        }
    };

    template<>
    struct __is_fast_hash<hash<napa::stl::String>> : std::false_type {
    };

    // std::hash specialization for napa::stl::U16String.
    template<>
    struct hash<napa::stl::U16String> : public __hash_base<size_t, napa::stl::U16String> {
        size_t operator()(const napa::stl::U16String& s) const noexcept {
            return std::_Hash_impl::hash(s.data(), s.length() * sizeof(char16_t));
        }
    };

    template<>
    struct __is_fast_hash<hash<napa::stl::U16String>> : std::false_type {
    };
}

#endif
