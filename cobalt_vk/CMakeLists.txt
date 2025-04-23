# VulkanEngine CMakeList.txt, "Author": alessandromanzini
# Engine specific logic here.
#
project("cobalt_vk")

# create library target
add_library(${PROJECT_NAME}
		src/ShaderModules.cpp
		src/Vertex.cpp
		src/UniformBufferObject.cpp
		include/UniformBufferObject.h
		include/SingleTimeCommand.h

		"src/assets/Buffer.cpp"
		"src/assets/Model.cpp"

		"src/builders/ModelLoader.cpp"
		"src/builders/BufferBuilder.cpp"

		"src/cleanup/DeletionQueue.cpp"
		"include/cleanup/Releasable.h"

        "src/queries/format_queries.cpp"
		"src/queries/memory_queries.cpp"
		"src/queries/swapchain_support_queries.cpp"
		"src/queries/queue_family_queries.cpp"

		"src/validation/result.cpp"
		"src/validation/dispatch.cpp"

		"include/views/BufferView.h"

		"src/ResourcePool.cpp"
		include/views/ModelView.h
)

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
		PUBLIC "./include/assets"
		PUBLIC "./include/builders"
		PUBLIC "./include/cleanup"
		PUBLIC "./include/validation"
		PUBLIC "./include/views"
		PRIVATE "./src"
		PRIVATE "./src/assets"
		PRIVATE "./src/builders"
		PRIVATE "./src/cleanup"
		PRIVATE "./src/validation" )

# fetch libraries
include(glfw_fetchcontent)
include(glm_fetchcontent)
include(assimp_fetchcontent)

target_link_libraries(${PROJECT_NAME}
		PUBLIC glfw
		PUBLIC glm
		PRIVATE assimp )

# require vulkan
include(vulkan_require)
