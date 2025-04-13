# shader compilation
set(COMPILE_SHADERS_COMMAND "compile_shaders")
set(SHADERS_DST_FOLDER "${CMAKE_CURRENT_BINARY_DIR}/shaders/")

# collect all shader files
file(GLOB MY_SHADERS "${SHADERS_SRC_FOLDER}/shader.*")

if (MY_SHADERS)
    # we use shell if we are on apple, bat if we are on windows
    if (APPLE)
        set(COMPILE_SCRIPT "${SHADERS_SRC_FOLDER}/compilers/compile.sh")
        add_custom_command(OUTPUT ${SHADERS_DST_FOLDER}
                COMMAND ${COMPILE_SCRIPT} -o ${SHADERS_DST_FOLDER} -i ${MY_SHADERS}
                DEPENDS ${MY_SHADERS}
                COMMENT "Re-compiling shaders")
    else ()
        set(COMPILE_SCRIPT "$ENV{VULKAN_SDK}/Bin/glslc")
        message(STATUS "Using glslc from Vulkan SDK: $ENV{VULKAN_SDK}/Bin/glslc")

        foreach (SHADER ${MY_SHADERS})
            get_filename_component(SHADER_NAME ${SHADER} NAME)
            set(OUTPUT_FILE "${SHADERS_DST_FOLDER}/${SHADER_NAME}.spv")
            add_custom_command(
                    OUTPUT ${OUTPUT_FILE}
                    COMMAND ${COMPILE_SCRIPT} -o ${OUTPUT_FILE} ${SHADER}
                    DEPENDS ${SHADER}
                    COMMENT "Compiling ${SHADER_NAME} to SPIR-V"
            )
            list(APPEND COMPILED_SHADERS ${OUTPUT_FILE})
        endforeach ()

        add_custom_target(${COMPILE_SHADERS_COMMAND} ALL
                DEPENDS ${COMPILED_SHADERS})
    endif ()

    add_custom_target(${COMPILE_SHADERS_COMMAND} ALL
            DEPENDS ${SHADERS_DST_FOLDER})

    # ensure your main project depends on the custom command
    add_dependencies(${PROJECT_NAME} ${COMPILE_SHADERS_COMMAND})

    message(STATUS "Shader compilation added as '${PROJECT_NAME}' dependency.")
endif ()
