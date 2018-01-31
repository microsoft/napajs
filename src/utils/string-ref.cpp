// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <napa/string-ref.h>

#include <codecvt>

namespace napa {
    namespace stl {
        // utility wrapper to adapt locale-bound facets for wstring/wbuffer convert
        template<class Facet>
        struct deletable_facet : Facet
        {
            template<class ...Args>
            deletable_facet(Args&& ...args) : Facet(std::forward<Args>(args)...) {}
            ~deletable_facet() {}
        };

#if (defined(_MSC_VER) && _MSC_VER < 2000)
        // The following code is a work-around of a known bug in VC14 (Visual Studio 2015/2017):
        // https://connect.microsoft.com/VisualStudio/feedback/details/1403302/unresolved-external-when-using-codecvt-utf8
        //
        template<>
        struct deletable_facet<std::codecvt_utf8_utf16<char16_t>>
            : deletable_facet<std::codecvt_utf8_utf16<uint16_t>>
        {
            inline result in(
                std::mbstate_t& state,
                const char *first1, const char *last1, const char *& mid1,
                char16_t *first2, char16_t *last2, char16_t *& mid2) const {

                return std::codecvt_utf8_utf16<uint16_t>::in(
                    state,
                    first1, last1, mid1,
                    reinterpret_cast<uint16_t *>(first2), reinterpret_cast<uint16_t *>(last2), reinterpret_cast<uint16_t *&>(mid2));
            }

            inline result out(
                std::mbstate_t& state,
                const char16_t *first1, const char16_t *last1, const char16_t *& mid1,
                char *first2, char *last2, char *& mid2) const {

                return std::codecvt_utf8_utf16<uint16_t>::out(
                    state,
                    reinterpret_cast<const uint16_t *>(first1), reinterpret_cast<const uint16_t *>(last1), reinterpret_cast<const uint16_t *&>(mid1),
                    first2, last2, mid2);
            }
        };
#endif

        typedef deletable_facet<std::codecvt_utf8_utf16<char16_t>> codecvt_utf16;

        inline std::string utf16_to_utf8(const char16_t *data, size_t size) {
            std::wstring_convert<codecvt_utf16, char16_t> cvt;

            return cvt.to_bytes(
                reinterpret_cast<const char16_t *>(data),
                reinterpret_cast<const char16_t *>(data) + size);
        }

        inline std::u16string utf8_to_utf16(const char *data, size_t size) {
            std::wstring_convert<codecvt_utf16, char16_t> cvt;
            
            return cvt.from_bytes(data, data + size);
        }
    }

    namespace string_ref {

        std::string ToString(const StringRef& str) {
            if (str.encoding == NAPA_STRING_ENCODING_UTF8 || str.encoding == NAPA_STRING_ENCODING_LATIN1) {
                return std::string(reinterpret_cast<const char *>(str.data), str.size);
            }

            // str.encoding == NAPA_STRING_ENCODING_UTF16
            return napa::stl::utf16_to_utf8(reinterpret_cast<const char16_t *>((str).data), (str).size);
        }

        std::u16string ToU16String(const StringRef& str) {
            if (str.encoding == NAPA_STRING_ENCODING_UTF8 || str.encoding == NAPA_STRING_ENCODING_LATIN1) {
                return napa::stl::utf8_to_utf16(reinterpret_cast<const char *>(str.data), str.size);
            }
            
            // str.encoding == NAPA_STRING_ENCODING_UTF16
            return std::u16string(reinterpret_cast<const char16_t *>(str.data), str.size);
        }
    }
}
