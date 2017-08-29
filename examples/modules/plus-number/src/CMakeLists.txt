execute_process(COMMAND node -e "require('napajs/build').paths.inc" RESULT_VARIABLE ERR OUTPUT_VARIABLE NAPAJS_INC)
if(ERR)
    message(FATAL_ERROR "Failed to resolve napa include directory")
endif(ERR)

execute_process(COMMAND node -e "require('napajs/build').paths.lib" RESULT_VARIABLE ERR OUTPUT_VARIABLE NAPAJS_LIB)
if(ERR)
    message(FATAL_ERROR "Failed to resolve napa library path")
endif(ERR)

#######################################################################################
# Build napa addon.
set(NAPA_ADDON_TARGET_NAME "${PROJECT_NAME}.napa")

# The generated library
add_library(${NAPA_ADDON_TARGET_NAME} SHARED "addon.cpp")

set_target_properties(${NAPA_ADDON_TARGET_NAME} PROPERTIES PREFIX "" SUFFIX "")

# Include directories
target_include_directories(${NAPA_ADDON_TARGET_NAME} PRIVATE
    ../plus-number
    ${CMAKE_JS_INC}
    ${NAPAJS_INC})

# Compiler definitions
target_compile_definitions(${NAPA_ADDON_TARGET_NAME} PRIVATE
    BUILDING_NAPA_EXTENSION)

# Link libraries
target_link_libraries(${NAPA_ADDON_TARGET_NAME} PRIVATE
    plus-number
    ${CMAKE_JS_LIB}
    ${NAPAJS_LIB})

#######################################################################################
# Build napa addon for node.
set(NODE_ADDON_TARGET_NAME "${PROJECT_NAME}.node")

# The generated library
add_library(${NODE_ADDON_TARGET_NAME} SHARED "addon.cpp")

set_target_properties(${NODE_ADDON_TARGET_NAME} PROPERTIES PREFIX "" SUFFIX "")

# Include directories
target_include_directories(${NODE_ADDON_TARGET_NAME} PRIVATE
    ../plus-number
    ${CMAKE_JS_INC}
    ${NAPAJS_INC})

# Compiler definitions
target_compile_definitions(${NODE_ADDON_TARGET_NAME} PRIVATE
    BUILDING_NODE_EXTENSION)

# Link libraries
target_link_libraries(${NODE_ADDON_TARGET_NAME} PRIVATE
    plus-number
    ${CMAKE_JS_LIB})
