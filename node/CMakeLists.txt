# Files to compile
# Do not napa core-modules that not needed in node isolation, like timer-wrap.cpp
file(GLOB SOURCE_FILES 
    "addon.cpp"
    "node-zone-delegates.cpp"
    "${PROJECT_SOURCE_DIR}/src/platform/filesystem.cpp"
    "${PROJECT_SOURCE_DIR}/src/platform/os.cpp"
    "${PROJECT_SOURCE_DIR}/src/zone/call-context.cpp"
    "${PROJECT_SOURCE_DIR}/src/zone/call-task.cpp"
    "${PROJECT_SOURCE_DIR}/src/zone/eval-task.cpp"
    "${PROJECT_SOURCE_DIR}/src/zone/terminable-task.cpp"
    "${PROJECT_SOURCE_DIR}/src/module/core-modules/napa/allocator-debugger-wrap.cpp"
    "${PROJECT_SOURCE_DIR}/src/module/core-modules/napa/allocator-wrap.cpp"
    "${PROJECT_SOURCE_DIR}/src/module/core-modules/napa/call-context-wrap.cpp"
    "${PROJECT_SOURCE_DIR}/src/module/core-modules/napa/lock-wrap.cpp"
    "${PROJECT_SOURCE_DIR}/src/module/core-modules/napa/metric-wrap.cpp"
    "${PROJECT_SOURCE_DIR}/src/module/core-modules/napa/napa-binding.cpp"
    "${PROJECT_SOURCE_DIR}/src/module/core-modules/napa/shared-ptr-wrap.cpp"
    "${PROJECT_SOURCE_DIR}/src/module/core-modules/napa/store-wrap.cpp"
    "${PROJECT_SOURCE_DIR}/src/module/core-modules/napa/transport-context-wrap-impl.cpp"
    "${PROJECT_SOURCE_DIR}/src/module/core-modules/napa/zone-wrap.cpp"        
    )

# The addon name
set(TARGET_NAME "${PROJECT_NAME}-binding")

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
    ${CMAKE_JS_INC}
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/module/core-modules/napa)

# Compiler definitions
target_compile_definitions(${TARGET_NAME} PRIVATE BUILDING_NODE_EXTENSION NAPA_BINDING_EXPORTS)

# Link libraries
target_link_libraries(${TARGET_NAME} PRIVATE
    ${PROJECT_NAME}
    ${CMAKE_JS_LIB})
