cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project("addon")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)

# Require Cxx14 features
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set symbol visibility to hidden by default.
# Napa shared library shares a few classes with napa-binding.node with different compile definition,
# exposing the same symbols from both shared libraries may cause improper behaviors under gcc.
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN)

# Prefix an suffix of Library name for different OS.
if (WIN32)
    set(LIBRARY_PREFIX "")
    set(LIBRARY_SUFFIX ".lib")
elseif (APPLE)
    set(LIBRARY_PREFIX "lib")
    set(LIBRARY_SUFFIX ".dylib")
else()
    set(LIBRARY_PREFIX "lib")
    set(LIBRARY_SUFFIX ".so")
endif()

execute_process(COMMAND node -e
    "var path = require('path'); process.stdout.write(path.resolve(path.dirname(require.resolve('napajs')), '..'));"
    RESULT_VARIABLE ERR
    OUTPUT_VARIABLE NAPAJS_ROOT)
if(ERR)
    message(FATAL_ERROR "Failed to get NAPAJS_ROOT")
endif(ERR)

#######################################################################################
# Build plus-number library.
set(LIBRARY_TARGET_NAME "plus-number")

# The generated library
add_library(${LIBRARY_TARGET_NAME} SHARED "src/plus-number.cpp")

# Include directories
target_include_directories(${LIBRARY_TARGET_NAME} PRIVATE inc)

# Compiler definitions
target_compile_definitions(${LIBRARY_TARGET_NAME} PRIVATE NAPA_EXAMPLE_API)

#######################################################################################
# Build napa addon.
set(NAPA_ADDON_TARGET_NAME "${PROJECT_NAME}.napa")

# The generated library
add_library(${NAPA_ADDON_TARGET_NAME} SHARED "src/addon.cpp")

set_target_properties(${NAPA_ADDON_TARGET_NAME} PROPERTIES PREFIX "" SUFFIX "")

# Include directories
target_include_directories(${NAPA_ADDON_TARGET_NAME} PRIVATE
    inc
    ${CMAKE_JS_INC}
    ${NAPAJS_ROOT}/inc)

# Compiler definitions
target_compile_definitions(${NAPA_ADDON_TARGET_NAME} PRIVATE
    NAPA_EXPORTS
    NAPA_BINDING_EXPORTS
    BUILDING_NAPA_EXTENSION)

# Link libraries
target_link_libraries(${NAPA_ADDON_TARGET_NAME} PRIVATE
    plus-number
    ${CMAKE_JS_LIB}
    ${NAPAJS_ROOT}/bin/${LIBRARY_PREFIX}napa${LIBRARY_SUFFIX})

#######################################################################################
if (CMAKE_JS_VERSION)
    # Build napa addon for node.
    set(NODE_ADDON_TARGET_NAME "${PROJECT_NAME}.node")

    # The generated library
    add_library(${NODE_ADDON_TARGET_NAME} SHARED "src/addon.cpp")

    set_target_properties(${NODE_ADDON_TARGET_NAME} PROPERTIES PREFIX "" SUFFIX "")

    # Include directories
    target_include_directories(${NODE_ADDON_TARGET_NAME} PRIVATE
        inc
        ${CMAKE_JS_INC}
        ${NAPAJS_ROOT}/inc)

    # Compiler definitions
    target_compile_definitions(${NODE_ADDON_TARGET_NAME} PRIVATE
        NAPA_BINDING_EXPORTS
        BUILDING_NODE_EXTENSION)

    # Link libraries
    target_link_libraries(${NODE_ADDON_TARGET_NAME} PRIVATE
        plus-number
        ${CMAKE_JS_LIB})
endif()
