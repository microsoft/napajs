cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project("napa")

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

# Build napa shared library.
# add_subdirectory(src)

set(CMAKE_CXX_FLAGS_ORG ${CMAKE_CXX_FLAGS})

# Use -fno-rtti and -fPIC to build v8-extensions as a static library for compatibility with node and v8.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fPIC")
add_subdirectory(src/v8-extensions)

# Restore CMAKE_CXX_FLAGS from CMAKE_CXX_FLAGS_ORG.
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_ORG})

if (CMAKE_JS_VERSION)
    # Build napa addon for node.
    add_subdirectory(node)
    # add_subdirectory(test/module/addon)
endif()
