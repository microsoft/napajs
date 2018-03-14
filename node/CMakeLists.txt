# Files to compile
file(GLOB SOURCE_FILES 
    "addon.cpp"
    "node-zone-delegates.cpp")

file(GLOB_RECURSE SOURCE_FILES_SRC
    ${PROJECT_SOURCE_DIR}/src/api/*.cpp
    ${PROJECT_SOURCE_DIR}/src/memory/*.cpp
    ${PROJECT_SOURCE_DIR}/src/module/*.cpp
    ${PROJECT_SOURCE_DIR}/src/platform/*.cpp
    ${PROJECT_SOURCE_DIR}/src/providers/*.cpp
    ${PROJECT_SOURCE_DIR}/src/settings/*.cpp
    ${PROJECT_SOURCE_DIR}/src/store/*.cpp
    ${PROJECT_SOURCE_DIR}/src/utils/*.cpp
    ${PROJECT_SOURCE_DIR}/src/zone/*.cpp
)

# The addon name
set(TARGET_NAME "${PROJECT_NAME}-binding")

# The generated library
add_library(${TARGET_NAME} SHARED ${SOURCE_FILES} ${SOURCE_FILES_SRC})

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


target_compile_definitions(${TARGET_NAME} PRIVATE USING_V8_SHARED)

# Compiler definitions
target_compile_definitions(${TARGET_NAME} PRIVATE BUILDING_NODE_EXTENSION NAPA_BINDING_EXPORTS)

# Link libraries
target_link_libraries(${TARGET_NAME} PRIVATE "v8-extensions")
