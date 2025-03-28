# VulkanEngine CMakeList.txt, "Author": alessandromanzini
# Engine specific logic here.
#
project("vulkan_engine")

# create library target
add_library(${PROJECT_NAME}
		src/TriangleApplication.cpp
        src/VulkanDeviceQueries.cpp
		src/ShaderModules.cpp
		src/Vertex.cpp
		src/UniformBufferObject.cpp
		include/UniformBufferObject.h
		include/SingleTimeCommand.h)

# set warning level to W4 and warnings as errors
if (MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
	target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# the include folder and resources has headers that will be shared to external project.
# the src folder also need to be included in case we have 
# headers for internal use.
target_include_directories(${PROJECT_NAME}
		PUBLIC "./include"
		PRIVATE "./src" )

# fetch libraries
include(glfw_fetchcontent)
include(glm_fetchcontent)
include(stb_fetchcontent)

target_link_libraries(${PROJECT_NAME}
		PUBLIC glfw
		PUBLIC glm
		PRIVATE stb )

# require vulkan
include(vulkan_require)

# Set the source directories for shaders
set(SHADERS_SRC_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/shaders" PARENT_SCOPE)
