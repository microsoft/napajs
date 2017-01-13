#ifndef NAPA_COMMON_C_H
#define NAPA_COMMON_C_H

#include "stddef.h"
#include "stdint.h"
#include "string.h"

/// <summary> Simple non ownning string. Should only be used for binding. </summary>
typedef struct {
    const char* data;
    size_t size;
} napa_string_ref;

typedef napa_string_ref NapaStringRef;

#define CREATE_NAPA_STRING_REF_WITH_SIZE(data, size) (napa_string_ref { (data), (size) })
#define CREATE_NAPA_STRING_REF(data) CREATE_NAPA_STRING_REF_WITH_SIZE(data, strlen(data))

#ifdef __cplusplus

#define STD_STRING_TO_NAPA_STRING_REF(str) (napa_string_ref { (str).data(), (str).size() })
#define NAPA_STRING_REF_TO_STD_STRING(str) (std::string((str).data, (str).size))

#endif // __cplusplus


#define NAPA_RESPONSE_CODE_DEF(symbol, ...) NAPA_RESPONSE_##symbol

typedef enum {

#include "napa/response-codes.inc"

} napa_response_code;

#undef NAPA_RESPONSE_CODE_DEF

typedef napa_response_code NapaResponseCode;

#endif // NAPA_COMMON_C_H
