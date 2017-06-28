# Files to compile
file(GLOB_RECURSE SOURCE_FILES "*.cpp")

# The target name
set(TARGET_NAME ${PROJECT_NAME})
    
# The generated library
add_library(${TARGET_NAME} SHARED ${SOURCE_FILES})

# Include directories
target_include_directories(${TARGET_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/third-party
    ${Boost_INCLUDE_DIRS}
    PUBLIC
    ${PROJECT_SOURCE_DIR}/inc)

# Compiler definitions
target_compile_definitions(${TARGET_NAME} PRIVATE NAPA_EXPORTS BUILDING_NAPA_EXTENSION NAPA_BINDING_EXPORTS)

# Link libraries
target_link_libraries(${TARGET_NAME} PRIVATE ${Boost_LIBRARIES})

if (CMAKE_JS_VERSION)
    # Building Napa as an npm package for node usage (using exported v8 from node.exe)

    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_JS_INC})
    target_link_libraries(${TARGET_NAME} PRIVATE ${CMAKE_JS_LIB})
    
    # Using the V8 functions exported from node.exe
    target_compile_definitions(${TARGET_NAME} PRIVATE USING_V8_SHARED)
else()
    # Building Napa for embed scenarios (static linking with v8)

    if (NOT DEFINED V8_INC)
        message(FATAL_ERROR "V8_INC must be set to the v8 include directory")
    endif()
    
    if (NOT DEFINED V8_LIB_PATH)
        message(FATAL_ERROR "V8_LIB_PATH must be set to the v8 libraries directory")
    endif()
    
    file(GLOB V8_LIBRARIES "${V8_LIB_PATH}/*.lib")
    
    target_include_directories(${TARGET_NAME} PRIVATE ${V8_INC})
    target_link_libraries(${TARGET_NAME} PRIVATE ${V8_LIBRARIES})

    # Using the V8 functions exported from node.exe
    target_compile_definitions(${TARGET_NAME} PRIVATE BUILDING_V8_SHARED)
endif()

if (WIN32)
    target_link_libraries(${TARGET_NAME} PRIVATE winmm.lib)
    
    # Define this to fix boost asio related warning ('Please define _WIN32_WINNT or _WIN32_WINDOWS appropriately')
    target_compile_definitions(${TARGET_NAME} PRIVATE _WIN32_WINNT=0x0501)
endif()
