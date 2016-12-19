///
/// Napa response codes definition file!
///
/// Guidelines:
///     1. Use NAPA_RESPONSE_CODE_DEF to define new codes.
///     2. Add new codes at the end of the list.
///     3. Make sure to add a comma at the end of the previous response code defintion
/// 
///    |----------------------- symbol name ---------- string representation --|
///     NAPA_RESPONSE_CODE_DEF( EXAMPLE_NAME1,        "Example string message1"),
///     NAPA_RESPONSE_CODE_DEF( EXAMPLE_NAME2,        "Example string message2")
/// Always add news codes
///

#ifndef NAPA_RESPONSE_CODE_DEF
#error NAPA_RESPONSE_CODE_DEF must be defined before including response_code.inc
#endif

NAPA_RESPONSE_CODE_DEF( SUCCESS,            "Success")
