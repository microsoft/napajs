cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project("simple-napa-addon")

set(NAPA_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)

# Require Cxx14 features
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The target name
set(TARGET_NAME ${PROJECT_NAME})

# The generated library
add_library(${TARGET_NAME} SHARED
    addon.cpp
    simple-object-wrap.cpp)

# Change output extension to 'napa'
set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "" SUFFIX ".napa")

# Set output directory for the addon
set_target_properties(${TARGET_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/build
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/build
    LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/build
    LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/build
)

# Include directories
target_include_directories(${TARGET_NAME}
    PRIVATE
    ${NAPA_ROOT}/inc
    ${CMAKE_JS_INC})

# Link libraries
target_link_libraries(${TARGET_NAME} PRIVATE ${CMAKE_JS_LIB})

# Link with napa shared library
if (WIN32)
    target_link_libraries(${TARGET_NAME} PRIVATE ${NAPA_ROOT}/bin/napa.lib)
elseif (APPLE)
    target_link_libraries(${TARGET_NAME} PRIVATE ${NAPA_ROOT}/bin/libnapa.dylib)
else()
    target_link_libraries(${TARGET_NAME} PRIVATE ${NAPA_ROOT}/bin/libnapa.so)
endif()

# Compiler definitions
target_compile_definitions(${TARGET_NAME} PRIVATE BUILDING_NAPA_EXTENSION)


