# Files to compile
file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# The target name
set(TARGET_NAME "napa-v8-extensions")

# The generated library
add_library(${TARGET_NAME} STATIC ${SOURCE_FILES})

# Include directories
target_include_directories(${TARGET_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/inc)

# Compiler definitions
target_compile_definitions(${TARGET_NAME} PRIVATE NAPA_EXPORTS NAPA_BINDING_EXPORTS NOMINMAX)

# Enable PIC
set_property(TARGET ${TARGET_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
# Disable RTTI
if(MSVC)
    target_compile_options(${TARGET_NAME} PRIVATE /GR-)
else()
    target_compile_options(${TARGET_NAME} PRIVATE -fno-rtti)
endif()

if(CMAKE_JS_VERSION)
    # Building Napa as an npm package for node usage (using exported v8 from node.exe)

    target_include_directories(${TARGET_NAME} PRIVATE
        ${CMAKE_JS_INC}
        ${PROJECT_SOURCE_DIR}/third-party/node)
else()
    # Building Napa for embed scenarios (static linking with v8)
    if (NOT DEFINED NODE_ROOT)
        message(FATAL_ERROR "NODE_ROOT must be set to the node sources root directory")
    endif()

    set(NODE_BUILD_TYPE "Release")
    if ((DEFINED CMAKE_BUILD_TYPE) AND (CMAKE_BUILD_TYPE STREQUAL "debug"))
        set(NODE_BUILD_TYPE "Debug")
    endif()

    # V8 header files
    target_include_directories(${TARGET_NAME} PRIVATE ${NODE_ROOT}/deps/v8/include)

endif()
