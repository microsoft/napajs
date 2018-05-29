cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project("napa-unittest")

set(NAPA_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Require Cxx14 features
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Test Files
file(GLOB_RECURSE TEST_FILES
    main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/settings/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/zone/*.cpp)

# Source files under test
file(GLOB_RECURSE SOURCE_FILES
    ${NAPA_ROOT}/src/platform/filesystem.cpp
    ${NAPA_ROOT}/src/platform/os.cpp
    ${NAPA_ROOT}/src/platform/process.cpp
    ${NAPA_ROOT}/src/settings/settings-parser.cpp
    ${NAPA_ROOT}/src/zone/simple-thread-pool.cpp
    ${NAPA_ROOT}/src/zone/timer.cpp)

# The target name
set(TARGET_NAME ${PROJECT_NAME})

# The generated test executable
add_executable(${TARGET_NAME} ${TEST_FILES} ${SOURCE_FILES} ${PLATFORM_SOURCE_FILES})

# Compiler definitions
target_compile_definitions(${TARGET_NAME} PRIVATE NAPA_LOG_DISABLED)

# Include directories
target_include_directories(${TARGET_NAME}
    PRIVATE
    ${NAPA_ROOT}/inc
    ${NAPA_ROOT}/src
    ${NAPA_ROOT}/third-party
    ${CMAKE_JS_INC})

# Set output directory for dll/libs
set_target_properties(${TARGET_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/build/test
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/build/test
)

# GCC/Clang: enable std::thread via -pthread option.
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(${TARGET_NAME} PRIVATE Threads::Threads)
endif()

# Copy module tests artifacts
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND
    ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/module/test-files ${CMAKE_CURRENT_SOURCE_DIR}/build/test)
