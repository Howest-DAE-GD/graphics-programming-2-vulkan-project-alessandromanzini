# Program CMakeList.txt, "Author": Manzini Alessandro
# Program specific logic here.
#
project("rasterizer")

# create program target WIN32
add_executable(${PROJECT_NAME}
		"src/main.cpp" )
		
# link library
target_link_libraries(${PROJECT_NAME}
		PRIVATE "vulkan_engine" )

# include program directories
target_include_directories(${PROJECT_NAME}
		PRIVATE "./include"
		PRIVATE "./src" )

# reload resources
include(reload_artifacts_command)

# shader compilation
include(shader_compilation_command)
