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

execute_process(COMMAND node -e "require('napajs/build').paths.inc" RESULT_VARIABLE ERR OUTPUT_VARIABLE NAPAJS_INC)
if(ERR)
    message(FATAL_ERROR "Failed to get NAPAJS_INC")
endif(ERR)

execute_process(COMMAND node -e "require('napajs/build').paths.lib" RESULT_VARIABLE ERR OUTPUT_VARIABLE NAPAJS_LIB)
if(ERR)
    message(FATAL_ERROR "Failed to get NAPAJS_LIB")
endif(ERR)

#######################################################################################
# Build plus-number library.
set(LIBRARY_TARGET_NAME "plus-number")

# The generated library
add_library(${LIBRARY_TARGET_NAME} STATIC "src/plus-number.cpp")

# Include directories
target_include_directories(${LIBRARY_TARGET_NAME} PRIVATE inc)

# Compiler definitions
target_compile_definitions(${LIBRARY_TARGET_NAME} PUBLIC NAPA_EXAMPLE_API)

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
    ${NAPAJS_INC})

# Compiler definitions
target_compile_definitions(${NAPA_ADDON_TARGET_NAME} PRIVATE
    NAPA_EXPORTS
    NAPA_BINDING_EXPORTS
    BUILDING_NAPA_EXTENSION)

# Link libraries
target_link_libraries(${NAPA_ADDON_TARGET_NAME} PRIVATE
    ${LIBRARY_TARGET_NAME}
    ${CMAKE_JS_LIB}
    ${NAPAJS_LIB})

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
        ${NAPAJS_INC})

    # Compiler definitions
    target_compile_definitions(${NODE_ADDON_TARGET_NAME} PRIVATE
        NAPA_BINDING_EXPORTS
        BUILDING_NODE_EXTENSION)

    # Link libraries
    target_link_libraries(${NODE_ADDON_TARGET_NAME} PRIVATE
        ${LIBRARY_TARGET_NAME}
        ${CMAKE_JS_LIB})
endif()
