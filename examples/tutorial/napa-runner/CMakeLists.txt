cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project("napa-runer")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)

# Require Cxx14 features
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(NAPAJS_ROOT ${PROJECT_SOURCE_DIR}/../../..)
set(NAPAJS_INC ${NAPAJS_ROOT}/inc)
find_library(NAPAJS_LIB NAMES napa PATHS ${NAPAJS_ROOT}/bin)

# A stable node version that can build as a library.
set(NODE_ROOT ${NAPAJS_ROOT}/build/node-v6.10.3)

set(BUILD_TYPE "Release")
if ((DEFINED CMAKE_BUILD_TYPE) AND (CMAKE_BUILD_TYPE STREQUAL "debug"))
    set(BUILD_TYPE "Debug")
endif()


set(TARGET_NAME "${PROJECT_NAME}")

add_executable(${TARGET_NAME} "main.cpp")

if("${CMAKE_SYSTEM}" MATCHES "Linux")

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")

    # Include directories
    target_include_directories(${TARGET_NAME} PRIVATE
    ${NODE_ROOT}/deps/v8/include
    ${NAPAJS_INC})

    # TODO #124: Remove dependencies of v8_libbase and icustubdata to embed Napa.js into a c++ program, because they are already dependencies to build napa shared library.
    find_library(V8_LIBBASE NAMES v8_libbase PATHS ${NODE_ROOT}/out/${BUILD_TYPE}/obj.target/deps/v8/tools/gyp)
    find_library(ICUSTUBDATA_LIB NAMES icustubdata PATHS ${NODE_ROOT}/out/${BUILD_TYPE}/obj.target/tools/icu)

    # Link libraries
    target_link_libraries(${TARGET_NAME} PRIVATE
    ${NAPAJS_LIB}
    ${CMAKE_DL_LIBS}
    ${V8_LIBBASE}
    ${ICUSTUBDATA_LIB}
    )

else()
    message(FATAL_ERROR "Build solution of this example is not provided for ${CMAKE_SYSTEM}")
endif()
