# Files to compile
file(GLOB SOURCE_FILES 
    "addon.cpp")

# The addon name
set(TARGET_NAME "${PROJECT_NAME}.node")

# The generated library
add_library(${TARGET_NAME} SHARED ${SOURCE_FILES})

set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "" SUFFIX "")

# Include directories
target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_JS_INC}
    ${PROJECT_SOURCE_DIR}/inc
    ${NAPAJS_ROOT}/inc)

# Compiler definitions
target_compile_definitions(${TARGET_NAME} PRIVATE NAPA_BINDING_EXPORTS BUILDING_NODE_EXTENSION)

# Link libraries
target_link_libraries(${TARGET_NAME} PRIVATE
    plus-number
    ${CMAKE_JS_LIB})
