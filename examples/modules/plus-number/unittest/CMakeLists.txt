cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project("library-test")

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

file(GLOB LIBRARY-FILES "../bin/*plus-number.*")
file(COPY ${LIBRARY-FILES} DESTINATION ${PROJECT_SOURCE_DIR}/bin/)

# Files to compile
file(GLOB SOURCE_FILES 
    "main.cpp")

# The executable name
set(TARGET_NAME "library-test")

# The generated library
add_executable(${TARGET_NAME} ${SOURCE_FILES})

# Include directories
target_include_directories(${TARGET_NAME} PRIVATE
    ../inc
    ${NAPAJS_ROOT}/third-party)

# Link libraries
target_link_libraries(${TARGET_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/bin/${LIBRARY_PREFIX}plus-number${LIBRARY_SUFFIX})