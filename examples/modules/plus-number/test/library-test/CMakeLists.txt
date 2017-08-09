# Files to compile
file(GLOB SOURCE_FILES 
    "main.cpp")

# The addon name
set(TARGET_NAME "plus-number-library-test")

# The generated library
add_executable(${TARGET_NAME} ${SOURCE_FILES})

# Include directories
target_include_directories(${TARGET_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/inc
    ${PROJECT_SOURCE_DIR}/node_modules/napajs/third-party)

# Link libraries
target_link_libraries(${TARGET_NAME} PRIVATE plus-number)

