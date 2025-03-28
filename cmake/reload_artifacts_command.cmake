# set artifacts reloading
set(RELOAD_COMMAND "reload_resources")

# set the source and destination directories for resources
set(RESOURCES_SRC_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/resources/")
set(RESOURCES_DST_FOLDER "${CMAKE_CURRENT_BINARY_DIR}/resources/")

# gather all artifacts from source directories
file(GLOB_RECURSE MY_RESOURCES "${RESOURCES_SRC_FOLDER}/*")

# define a custom command to copy artifacts
# this command depends on the files in RESOURCES_SRC_FOLDER
add_custom_command( OUTPUT ${RESOURCES_DST_FOLDER}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${RESOURCES_DST_FOLDER}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MY_RESOURCES} ${RESOURCES_DST_FOLDER}
        DEPENDS ${MY_RESOURCES}
        COMMENT "Reloading resources." )

add_custom_target(${RELOAD_COMMAND} ALL
        DEPENDS ${RESOURCES_DST_FOLDER} )

# ensure your main project depends on the custom command
add_dependencies(${PROJECT_NAME} ${RELOAD_COMMAND})

message( STATUS "Artifacts reloading added as '${PROJECT_NAME}' dependency." )
