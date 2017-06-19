# Files to compile
file(GLOB SOURCE_FILES "addon.cpp" "../src/module/core-modules/napa/*.cpp")

# The addon name
set(TARGET_NAME "${PROJECT_NAME}-binding")

# The generated library
add_library(${TARGET_NAME} SHARED ${SOURCE_FILES})

set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

# Include directories
target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_JS_INC}
    ${Boost_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/src/module/core-modules/napa)

# Compiler definitions
target_compile_definitions(${TARGET_NAME} PRIVATE BUILDING_NODE_EXTENSION)

# Link libraries
target_link_libraries(${TARGET_NAME} PRIVATE
    ${PROJECT_NAME}
    ${Boost_LIBRARIES}
    ${CMAKE_JS_LIB})
