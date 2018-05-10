cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project("napa")

option(DEBUG_NAPA "build with more deep bebugging macro")
message(STATUS "    --MACRO: DEBUG_NAPA=${DEBUG_NAPA}")
if (DEBUG_NAPA)
    add_compile_options(-DDEBUG_NAPA)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)

# Require Cxx14 features
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT WIN32 AND NOT APPLE)
    # Set symbol visibility to hidden by default.
    # Napa shared library shares a few classes with napa-binding.node with different compile definition,
    # exposing the same symbols from both shared libraries may cause improper behaviors under gcc.
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    set(CMAKE_VISIBILITY_INLINES_HIDDEN)

    # Prevent symbol relocations internal to our wrapper library to be overwritten by the application.
    set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-Bsymbolic -Wl,-Bsymbolic-functions")
    set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-Bsymbolic -Wl,-Bsymbolic-functions")

    # Mark object non-lazy runtime binding.
    set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,now")
    set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-z,now")
endif ()

# Static link VCRuntime
if (MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

# Build v8-extensions as a static library.
# Use -fno-rtti and -fPICfor for compatibility with node and v8.
# Restore them for other targets.
set(CMAKE_CXX_FLAGS_ORG ${CMAKE_CXX_FLAGS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fPIC")
add_subdirectory(src/v8-extensions)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_ORG})

if (CMAKE_JS_VERSION)
    # Build napa addon for node.
    add_subdirectory(node)
    add_subdirectory(test/module/addon)
endif()
