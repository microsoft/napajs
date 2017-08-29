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
    NAPA_EXPORTS
    NAPA_BINDING_EXPORTS
    BUILDING_NAPA_EXTENSION)

# Link libraries
target_link_libraries(${NAPA_ADDON_TARGET_NAME} PRIVATE
    plus-number
    ${CMAKE_JS_LIB}
    ${NAPAJS_LIB})

#######################################################################################
if (CMAKE_JS_VERSION)
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
        NAPA_BINDING_EXPORTS
        BUILDING_NODE_EXTENSION)

    # Link libraries
    target_link_libraries(${NODE_ADDON_TARGET_NAME} PRIVATE
        plus-number
        ${CMAKE_JS_LIB})
endif()
