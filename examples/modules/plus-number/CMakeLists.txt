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

execute_process(COMMAND cmd /c "node -e process.stdout.write(require.resolve('napajs'))" RESULT_VARIABLE ERR OUTPUT_VARIABLE NAPAJS_PATH)
if(ERR)
    message(FATAL_ERROR "Failed to get NAPAJS_PATH")
endif(ERR)
string(REGEX MATCH ^.*napajs NAPAJS_ROOT ${NAPAJS_PATH})

# Build plus-number library.
add_subdirectory(src)

# Build napa addon.
add_subdirectory(napa)

if (CMAKE_JS_VERSION)
    # Build napa addon for node.
    add_subdirectory(node)
endif()

# Build plus-number library.
add_subdirectory(test/library-test)
