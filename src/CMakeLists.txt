# Files to compile
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# The target name
set(TARGET_NAME ${PROJECT_NAME})

# The generated library
add_library(${TARGET_NAME} SHARED ${SOURCE_FILES})

# Include directories
target_include_directories(${TARGET_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/third-party
    PUBLIC
    ${PROJECT_SOURCE_DIR}/inc)

# Compiler definitions
target_compile_definitions(${TARGET_NAME} PRIVATE NAPA_EXPORTS NAPA_BINDING_EXPORTS BUILDING_NAPA_EXTENSION)

# Link libraries
target_link_libraries(${TARGET_NAME} PRIVATE)

if (CMAKE_JS_VERSION)
    # Building Napa as an npm package for node usage (using exported v8 from node.exe)

    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_JS_INC})
    target_link_libraries(${TARGET_NAME} PRIVATE ${CMAKE_JS_LIB})

    # Using the V8 functions exported from node.exe
    target_compile_definitions(${TARGET_NAME} PRIVATE USING_V8_SHARED)
else()
    # Building Napa for embed scenarios (static linking with v8)

    if (NOT DEFINED NODE_ROOT)
        message(FATAL_ERROR "NODE_ROOT must be set to the node sources root directory")
    endif()

    set(NODE_BUILD_TYPE "Release")
    if ((DEFINED CMAKE_BUILD_TYPE) AND (CMAKE_BUILD_TYPE STREQUAL "debug"))
        set(NODE_BUILD_TYPE "Debug")
    endif()

    find_library(V8_BASE_0_LIBRARY NAMES v8_base_0 PATHS ${NODE_ROOT}/build/${NODE_BUILD_TYPE}/lib)
    find_library(V8_BASE_1_LIBRARY NAMES v8_base_1 PATHS ${NODE_ROOT}/build/${NODE_BUILD_TYPE}/lib)
    find_library(V8_BASE_2_LIBRARY NAMES v8_base_2 PATHS ${NODE_ROOT}/build/${NODE_BUILD_TYPE}/lib)
    find_library(V8_BASE_3_LIBRARY NAMES v8_base_3 PATHS ${NODE_ROOT}/build/${NODE_BUILD_TYPE}/lib)
    find_library(V8_LIBBASE_LIBRARY NAMES v8_libbase PATHS ${NODE_ROOT}/build/${NODE_BUILD_TYPE}/lib)
    find_library(V8_LIBPLATFORM_LIBRARY NAMES v8_libplatform PATHS ${NODE_ROOT}/build/${NODE_BUILD_TYPE}/lib)
    find_library(V8_NOSNAPSHOT_LIBRARY NAMES v8_nosnapshot PATHS ${NODE_ROOT}/build/${NODE_BUILD_TYPE}/lib)

    find_library(ICUI18N_LIBRARY NAMES icui18n PATHS ${NODE_ROOT}/${NODE_BUILD_TYPE}/lib)
    find_library(ICUSTUBDATA_LIBRARY NAMES icustubdata PATHS ${NODE_ROOT}/${NODE_BUILD_TYPE}/lib)
    find_library(ICUUCX_LIBRARY NAMES icuucx PATHS ${NODE_ROOT}/${NODE_BUILD_TYPE}/lib)

    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS "/LTCG")

    # V8 header files
    target_include_directories(${TARGET_NAME} PRIVATE ${NODE_ROOT}/deps/v8/include)

    # V8 static libraries
    target_link_libraries(${TARGET_NAME} PRIVATE
        ${V8_BASE_0_LIBRARY}
        ${V8_BASE_1_LIBRARY}
        ${V8_BASE_2_LIBRARY}
        ${V8_BASE_3_LIBRARY}
        ${V8_LIBBASE_LIBRARY}
        ${V8_LIBPLATFORM_LIBRARY}
        ${V8_NOSNAPSHOT_LIBRARY}
        ${ICUI18N_LIBRARY}
        ${ICUSTUBDATA_LIBRARY}
        ${ICUUCX_LIBRARY})
endif()

if (WIN32)
    target_link_libraries(${TARGET_NAME} PRIVATE winmm.lib)
endif()
