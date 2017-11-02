cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project("estimate-PI-with-embeded-napa")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)

# Require Cxx14 features
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(NAPAJS_INC ${PROJECT_SOURCE_DIR}/../../../inc)
set(NAPAJS_LIB ${PROJECT_SOURCE_DIR}/../../../bin/libnapa.so)

set(TARGET_NAME "${PROJECT_NAME}")

add_executable(${TARGET_NAME} "main.cpp")

if("${CMAKE_SYSTEM}" MATCHES "Linux")

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")

    # Include directories
    target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_JS_INC}
    ${NAPAJS_INC})

    # Link libraries
    target_link_libraries(${TARGET_NAME} PRIVATE
    ${CMAKE_JS_LIB}
    ${NAPAJS_LIB}
    ${CMAKE_DL_LIBS}
    ${PROJECT_SOURCE_DIR}/../../../build/node-v6.10.3/out/Release/obj.target/deps/v8/tools/gyp/libv8_libbase.a
    ${PROJECT_SOURCE_DIR}/../../../build/node-v6.10.3/out/Release/obj.target/tools/icu/libicustubdata.a
    )

else()
    message(FATAL_ERROR "Build solution of this example is not provided for ${CMAKE_SYSTEM}")
endif()
