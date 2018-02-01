// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "napa/exports.h"

#include "stddef.h"
#include "stdint.h"

#ifdef __cplusplus
#include <string>
#include <cstring>
#endif

typedef enum {

    /// <summary> Represent a UTF-8 encoding string. Use 8-bit 'char' as the char type. </summary>
    NAPA_STRING_ENCODING_UTF8,
    /// <summary> Represent a Latin-1 encoding string. Use 8-bit 'char' as the char type. </summary>
    NAPA_STRING_ENCODING_LATIN1,
    /// <summary> Represent a UTF-16 encoding string. Use 16-bit 'char16_t' as the char type. </summary>
    NAPA_STRING_ENCODING_UTF16

} napa_string_encoding;

typedef void napa_string_char_t;

/// <summary> Simple non owning string. Should only be used for binding. </summary>
/// <remarks>
/// There are 3 types of supported encoding as listed below:
/// - UTF-8. Use 8-bit 'char' as the char type.
/// - Latin-1. Use 8-bit 'char' as the char type.
/// - UTF-16. Use 16-bit 'char16_t' as the char type.
/// </remarks>
typedef struct napa_string_ref {
    /// <summary> A pointer to the C-style string buffer. The char type should match its encoding. </summary>
    const napa_string_char_t* data;
    /// <summary> Specify the count of chars in the string buffer. </summary>
    /// <remarks>
    /// This field does not indicate the number of characters in the string. For example, The euro sign (U+20AC)
    /// is one character that takes 3 chars to represent in UTF-8 encoding.
    /// </remarks>
    size_t size;
    /// <summary> Specify the string encoding. </summary>
    napa_string_encoding encoding;

#ifdef __cplusplus
    inline static napa_string_ref FromUtf8(const char * data, size_t size);
    inline static napa_string_ref FromUtf8(const char * data);
    inline static napa_string_ref FromUtf8(const std::string & str);

    inline static napa_string_ref FromLatin1(const char * data, size_t size);
    inline static napa_string_ref FromLatin1(const char * data);
    inline static napa_string_ref FromLatin1(const std::string & str);

    inline static napa_string_ref FromUtf16(const char16_t * data, size_t size);
    inline static napa_string_ref FromUtf16(const char16_t * data);
    inline static napa_string_ref FromUtf16(const std::u16string & str);

    inline std::string ToString() const;
    inline std::u16string ToU16String() const;
#endif

} napa_string_ref;

#define NAPA_STRING_REF_WITH_SIZE(data, size, encoding) (napa_string_ref { (data), (size), (encoding) })

const napa_string_ref EMPTY_NAPA_STRING_REF = NAPA_STRING_REF_WITH_SIZE(0, 0, NAPA_STRING_ENCODING_UTF8);

#ifdef __cplusplus

namespace napa {
    typedef napa_string_ref StringRef;

    namespace string_ref {
        NAPA_API std::string ToString(const StringRef& str);
        NAPA_API std::u16string ToU16String(const StringRef& str);
    }
}

#define NAPA_STRING_REF_WITH_SIZE_LATIN1(data, size) NAPA_STRING_REF_WITH_SIZE(data, size, NAPA_STRING_ENCODING_LATIN1)
#define NAPA_STRING_REF_WITH_SIZE_UTF8(data, size) NAPA_STRING_REF_WITH_SIZE(data, size, NAPA_STRING_ENCODING_UTF8)
#define NAPA_STRING_REF_WITH_SIZE_UTF16(data, size) NAPA_STRING_REF_WITH_SIZE(data, size, NAPA_STRING_ENCODING_UTF16)

#define NAPA_STRING_REF_LATIN1(data) NAPA_STRING_REF_WITH_SIZE_LATIN1(data, std::strlen(data))
#define NAPA_STRING_REF_UTF8(data) NAPA_STRING_REF_WITH_SIZE_UTF8(data, std::strlen(data))
#define NAPA_STRING_REF_UTF16(data) NAPA_STRING_REF_WITH_SIZE_UTF16(data, std::char_traits<char16_t>::length(data))

#define STD_STRING_TO_NAPA_STRING_REF_LATIN1(str) (napa_string_ref { (str).data(), (str).size(), NAPA_STRING_ENCODING_LATIN1 })
#define STD_STRING_TO_NAPA_STRING_REF_UTF8(str) (napa_string_ref { (str).data(), (str).size(), NAPA_STRING_ENCODING_UTF8 })
#define STD_U16STRING_TO_NAPA_STRING_REF(str) (napa_string_ref { (str).data(), (str).size(), NAPA_STRING_ENCODING_UTF16 })


/* inline static */ napa_string_ref napa_string_ref::FromUtf8(const char * data, size_t size) {
    return NAPA_STRING_REF_WITH_SIZE_UTF8(data, size);
}
/* inline static */ napa_string_ref napa_string_ref::FromUtf8(const char * data) {
    return NAPA_STRING_REF_UTF8(data);
}
/* inline static */ napa_string_ref napa_string_ref::FromUtf8(const std::string & str) {
    return STD_STRING_TO_NAPA_STRING_REF_UTF8(str);
}

/* inline static */ napa_string_ref napa_string_ref::FromLatin1(const char * data, size_t size) {
    return NAPA_STRING_REF_WITH_SIZE_LATIN1(data, size);
}
/* inline static */ napa_string_ref napa_string_ref::FromLatin1(const char * data) {
    return NAPA_STRING_REF_LATIN1(data);
}
/* inline static */ napa_string_ref napa_string_ref::FromLatin1(const std::string & str) {
    return STD_STRING_TO_NAPA_STRING_REF_LATIN1(str);
}

/* inline static */ napa_string_ref napa_string_ref::FromUtf16(const char16_t * data, size_t size) {
    return NAPA_STRING_REF_WITH_SIZE_UTF16(data, size);
}
/* inline static */ napa_string_ref napa_string_ref::FromUtf16(const char16_t * data) {
    return NAPA_STRING_REF_UTF16(data);
}
/* inline static */ napa_string_ref napa_string_ref::FromUtf16(const std::u16string & str) {
    return STD_U16STRING_TO_NAPA_STRING_REF(str);
}


#define NAPA_STRING_REF_TO_STD_STRING(ref) napa::string_ref::ToString(ref)
#define NAPA_STRING_REF_TO_STD_U16STRING(ref) napa::string_ref::ToU16String(ref)

/* inline */ std::string napa_string_ref::ToString() const {
    return NAPA_STRING_REF_TO_STD_STRING(*this);
}

/* inline */ std::u16string napa_string_ref::ToU16String() const {
    return NAPA_STRING_REF_TO_STD_U16STRING(*this);
}

#endif // __cplusplus

