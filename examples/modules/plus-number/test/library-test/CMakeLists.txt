# Files to compile
file(GLOB SOURCE_FILES 
    "main.cpp")

# The addon name
set(TARGET_NAME "library-test")

# The generated library
add_executable(${TARGET_NAME} ${SOURCE_FILES})

# Include directories
target_include_directories(${TARGET_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/inc
    ${NAPAJS_ROOT}/third-party)

# Link libraries
target_link_libraries(${TARGET_NAME} PRIVATE plus-number)
