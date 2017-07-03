cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project("napa")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)

# Find boost
set(REQUIRED_BOOST_COMPONENTS "filesystem;system;thread")
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
if (CMAKE_JS_VERSION)
    # Include BoostLib module

    if(WIN32)
        execute_process(COMMAND cmd /c "npm prefix -g" RESULT_VARIABLE ERR OUTPUT_VARIABLE NPM_GLOBAL_PATH)
        if(ERR)
            message(FATAL_ERROR "Failed to get npm global installation path. ${ERR}, ${NPM_GLOBAL_PATH}")
        endif(ERR)
        string(REPLACE "\\" "/" NPM_GLOBAL_PATH ${NPM_GLOBAL_PATH})
        string(STRIP ${NPM_GLOBAL_PATH} NPM_GLOBAL_PATH)
    else()
        # TODO: extract NPM_GLOBAL PATH in non-windows environments
    endif()

    list(APPEND CMAKE_MODULE_PATH "${NPM_GLOBAL_PATH}/node_modules/boost-lib/cmake")
    include(BoostLib)
    
    require_boost_libs(">= 1.62.0" "${REQUIRED_BOOST_COMPONENTS}")
else()
    find_package(Boost 1.62.0 REQUIRED "${REQUIRED_BOOST_COMPONENTS}")
    if(NOT Boost_FOUND)
        message(FATAL_ERROR "Could not find the boost package. Try setting BOOST_ROOT variable")
    endif()
endif()

# Build napa shared library.
add_subdirectory(src)

if (CMAKE_JS_VERSION)
    # Build napa addon for node.
    add_subdirectory(node)
endif()
