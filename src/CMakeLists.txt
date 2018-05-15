# Files to compile
file(GLOB_RECURSE SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/api/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/memory/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/module/module.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/module/core-modules/napa/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/node/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platform/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/providers/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/settings/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/store/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/zone/*.cpp
)

# The addon name
set(TARGET_NAME ${PROJECT_NAME})

# The generated library
add_library(${TARGET_NAME} SHARED ${SOURCE_FILES})

set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

# Rpath definitions

if (APPLE)
    set_target_properties(${TARGET_NAME} PROPERTIES INSTALL_RPATH "@loader_path")
else ()
    set_target_properties(${TARGET_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN/")
endif()

set_target_properties(${TARGET_NAME} PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH_USE_LINK_PATH FALSE)

# Include directories
target_include_directories(${TARGET_NAME} PRIVATE
     "/usr/local/include/node"
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/module/core-modules/napa
    ${PROJECT_SOURCE_DIR}/third-party
    PUBLIC
    ${PROJECT_SOURCE_DIR}/inc)

# Compiler definitions
target_compile_definitions(${TARGET_NAME}
    PRIVATE NAPA_EXPORTS NAPA_BINDING_EXPORTS NOMINMAX $<$<CONFIG:Debug>:NAPA_DEBUG_ENABLED>)

# Link libraries
target_link_libraries(${TARGET_NAME} PRIVATE "napa-v8-extensions" ${CMAKE_JS_LIB})
