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
    message(FATAL_ERROR "Failed to resolve napa include directory")
endif(ERR)

execute_process(COMMAND node -e "require('napajs/build').paths.lib" RESULT_VARIABLE ERR OUTPUT_VARIABLE NAPAJS_LIB)
if(ERR)
    message(FATAL_ERROR "Failed to resolve napa library path")
endif(ERR)

# Build plus-number library
add_subdirectory(plus-number)

# Build napa addon
add_subdirectory(src)
