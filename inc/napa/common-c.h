#ifndef NAPA_COMMON_C_H
#define NAPA_COMMON_C_H

#include "stddef.h"
#include "stdint.h"

/// <summary>Simple non ownning string. Should only be used for binding.</summary>
typedef struct {
    const char* data;
    size_t size;
} napa_string_ref;

#ifdef __cplusplus

#define CREATE_NAPA_STRING_REF(data, size) (napa_string_ref { (data), (size) })
#define STD_STRING_TO_NAPA_STRING_REF(str) (napa_string_ref { (str).data(), (str).size() })
#define NAPA_STRING_REF_TO_STD_STRING(str) (std::string((str).data, (str).size))

#endif // __cplusplus


#define NAPA_RESPONSE_CODE_DEF(symbol, ...) NAPA_RESPONSE_##symbol

typedef enum {

#include "napa/response-codes.inc"

} NapaResponseCode;

#undef NAPA_RESPONSE_CODE_DEF

#endif // NAPA_COMMON_C_H
