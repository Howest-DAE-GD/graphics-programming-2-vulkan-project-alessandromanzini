# shader compilation
set(COMPILE_SHADERS_COMMAND "compile_shaders")

# Set the src and dest directories for shaders
set(SHADERS_SRC_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/shaders/")
set(SHADERS_DST_FOLDER "${CMAKE_CURRENT_BINARY_DIR}/shaders/")

# collect all shader files
file(GLOB MY_SHADERS "${SHADERS_SRC_FOLDER}/*.*")

if (MY_SHADERS)
    get_filename_component(GLSLC "$ENV{VULKAN_SDK}/bin/glslc" ABSOLUTE)

    set(SHADER_OUTPUTS "")

    foreach(SRC IN LISTS MY_SHADERS)
        get_filename_component(FILENAME ${SRC} NAME)
        set(SPV "${SHADERS_DST_FOLDER}/${FILENAME}.spv")
        list(APPEND SHADER_OUTPUTS ${SPV})

        add_custom_command(
                OUTPUT ${SPV}
                COMMAND ${GLSLC} ${SRC} -o ${SPV}
                DEPENDS ${SRC}
                COMMENT "Compiling shader: ${FILENAME}"
                VERBATIM )
    endforeach()

    add_custom_target(${COMPILE_SHADERS_COMMAND} ALL
            DEPENDS ${SHADER_OUTPUTS})

    # ensure your main project depends on the custom command
    add_dependencies(${PROJECT_NAME} ${COMPILE_SHADERS_COMMAND})

    message(STATUS "Shader compilation added as '${PROJECT_NAME}' dependency.")
endif ()
