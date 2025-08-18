# Program CMakeList.txt, "Author": alessandromanzini
# Program specific logic here.
#
project("application")

# create program target WIN32
add_executable(${PROJECT_NAME}
		"code/main.cpp"
		"code/MyApplication.cpp"
		"code/debug_callback.h"
		"code/Camera.cpp"
		"code/Timer.cpp"
		code/light.cpp
		code/light.h)

# set warning level to W4 and warnings as errors
if (MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
	target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# include program directories
target_include_directories(${PROJECT_NAME}
		PRIVATE "./include"
		PRIVATE "./src" )

# include stb
include(stb_fetchcontent)

# link library
target_link_libraries(${PROJECT_NAME}
		PRIVATE "cobalt"
		PRIVATE "xos" )
target_compile_definitions("cobalt" PRIVATE COBALT_ENABLE_COMMAND_POOL_LOGGING)

# set the correct relative path for the executable
if(APPLE)
	set_target_properties(${PROJECT_NAME} PROPERTIES BUILD_RPATH "@executable_path" )
endif()

# copy the dynamic libraries to the output directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:xos> $<TARGET_FILE_DIR:${PROJECT_NAME}> )

# reload resources
include(reload_artifacts_command)

# shader compilation
include(shader_compilation_command)
