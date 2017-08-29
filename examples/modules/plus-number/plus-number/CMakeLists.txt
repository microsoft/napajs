# Build plus-number library.
set(TARGET_NAME "plus-number")

# The generated library
add_library(${TARGET_NAME} SHARED "plus-number.cpp")

# Compiler definitions
target_compile_definitions(${TARGET_NAME} PRIVATE NAPA_EXAMPLE_API)
