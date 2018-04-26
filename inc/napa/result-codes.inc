// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

///
/// Napa result codes definition file!
///
/// Guidelines:
///     1. Use NAPA_RESULT_CODE_DEF to define new codes.
///     2. Add new codes at the end of the list.
///     3. Make sure to add a comma at the end of the previous result code defintion
/// 
///    |----------------------- symbol name ---------- string representation --|
///     NAPA_RESULT_CODE_DEF( EXAMPLE_NAME1,        "Example string message1"),
///     NAPA_RESULT_CODE_DEF( EXAMPLE_NAME2,        "Example string message2")
/// Always add news codes
///

#ifndef NAPA_RESULT_CODE_DEF
#error NAPA_RESULT_CODE_DEF must be defined before including response_code.inc
#endif

NAPA_RESULT_CODE_DEF( SUCCESS,                         "Success"),
NAPA_RESULT_CODE_DEF( UNDEFINED,                       "Undefined"),
NAPA_RESULT_CODE_DEF( INTERNAL_ERROR,                  "Napa internal error"),
NAPA_RESULT_CODE_DEF( TIMEOUT,                         "The request timed out"),
NAPA_RESULT_CODE_DEF( ZONE_INIT_ERROR,                 "Failed to initialize zone"),
NAPA_RESULT_CODE_DEF( ZONE_BROADCAST_ERROR,            "Failed to broadcast JavaScript request in zone"),
NAPA_RESULT_CODE_DEF( ZONE_EXECUTE_ERROR,              "Failed to execute the JavaScript request"),
NAPA_RESULT_CODE_DEF( ZONE_IS_RECYCLING,               "The specific zone is being recycling"),
NAPA_RESULT_CODE_DEF( SETTINGS_PARSER_ERROR,           "Failed to parse settings"),
NAPA_RESULT_CODE_DEF( PROVIDERS_INIT_ERROR,            "Failed to initialize providers"),
NAPA_RESULT_CODE_DEF( V8_INIT_ERROR,                   "Failed to initialize V8"),
NAPA_RESULT_CODE_DEF( GLOBAL_VALUE_ERROR,              "Failed to set global value")
