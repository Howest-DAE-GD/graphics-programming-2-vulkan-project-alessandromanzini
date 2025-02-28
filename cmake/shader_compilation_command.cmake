# shader compilation
set(COMPILE_SHADERS_COMMAND "compile_shaders")
set(SHADERS_DST_FOLDER "${CMAKE_CURRENT_BINARY_DIR}/shaders")
file(GLOB MY_SHADERS "${SHADERS_SRC_FOLDER}/shader.*")

if(MY_SHADERS)
    # we use shell if we are on apple, bat if we are on windows
    if(APPLE)
        set(COMPILE_SCRIPT "${SHADERS_SRC_FOLDER}/compile.sh")
        add_custom_command( OUTPUT ${SHADERS_DST_FOLDER}
                COMMAND ${COMPILE_SCRIPT} -o ${SHADERS_DST_FOLDER} -i ${MY_SHADERS}
                DEPENDS ${MY_SHADERS}
                COMMENT "Re-compiling shaders" )
    else()
    endif()

    add_custom_target(${COMPILE_SHADERS_COMMAND} ALL
            DEPENDS ${SHADERS_DST_FOLDER} )

    # ensure your main project depends on the custom command
    add_dependencies(${PROJECT_NAME} ${COMPILE_SHADERS_COMMAND})

    message( STATUS "Shader compilation added as '${PROJECT_NAME}' dependency." )
endif()
