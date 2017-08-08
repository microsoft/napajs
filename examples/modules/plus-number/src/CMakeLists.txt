# Files to compile
file(GLOB SOURCE_FILES 
    "plus-number.cpp")

# The addon name
set(TARGET_NAME "plus-number")

# The generated library
add_library(${TARGET_NAME} SHARED ${SOURCE_FILES})

# Include directories
target_include_directories(${TARGET_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/inc)

# Compiler definitions
target_compile_definitions(${TARGET_NAME} PRIVATE NAPA_EXAMPLE_API)