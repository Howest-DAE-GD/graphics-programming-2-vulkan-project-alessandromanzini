# shader compilation
set(COMPILE_SHADERS_COMMAND "compile_shaders")

# Set the src and dest directories for shaders
set(SHADERS_SRC_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/shaders/")
set(SHADERS_DST_FOLDER "${CMAKE_CURRENT_BINARY_DIR}/shaders/")

# collect all shader files
file(GLOB ALL_SHADER_FILES "${SHADERS_SRC_FOLDER}/*.*")

# filter out files matching */common.*.*
set(MY_SHADERS "")
foreach (F ${ALL_SHADER_FILES})
    if (NOT F MATCHES ".*/common\\..*\\..*")
        list(APPEND MY_SHADERS ${F})
    endif ()
endforeach ()

# print the shaders found
list(LENGTH MY_SHADERS SHADER_COUNT)
if (${SHADER_COUNT} GREATER 0)
    message(STATUS "Watching shader files:")
    foreach (F ${MY_SHADERS})
        get_filename_component(PARENT_DIR "${F}" DIRECTORY)
        get_filename_component(LAST_FOLDER "${PARENT_DIR}" NAME)
        get_filename_component(FILE_NAME "${F}" NAME)
        message(STATUS "    ${LAST_FOLDER}/${FILE_NAME}")
    endforeach ()
endif ()

if (MY_SHADERS)
    get_filename_component(GLSLC "$ENV{VULKAN_SDK}/bin/glslc" ABSOLUTE)

    set(SHADER_OUTPUTS "")

    foreach (SRC IN LISTS MY_SHADERS)
        get_filename_component(FILENAME ${SRC} NAME)
        set(SPV "${SHADERS_DST_FOLDER}/${FILENAME}.spv")
        list(APPEND SHADER_OUTPUTS ${SPV})

        add_custom_command(
                OUTPUT ${SPV}
                COMMAND ${GLSLC} ${SRC} -o ${SPV}
                DEPENDS ${SRC}
                COMMENT "Compiling shader: ${FILENAME}"
                VERBATIM)
    endforeach ()

    add_custom_target(${COMPILE_SHADERS_COMMAND} ALL DEPENDS ${SHADER_OUTPUTS})

    # ensure your main project depends on the custom command
    add_dependencies(${PROJECT_NAME} ${COMPILE_SHADERS_COMMAND})

    message(STATUS "Shader compilation added as '${PROJECT_NAME}' dependency.")
endif ()
