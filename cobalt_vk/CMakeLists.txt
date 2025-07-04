# VulkanEngine CMakeList.txt, "Author": alessandromanzini
# Engine specific logic here.
#
project("cobalt_vk")

# create library target
add_library(${PROJECT_NAME}
        src/VulkanDeviceQueries.cpp
		src/ShaderModules.cpp
		src/UniformBufferObject.cpp
		include/public/UniformBufferObject.h
		include/public/SingleTimeCommand.h

		"src/assets/Buffer.cpp"
		"src/assets/Model.cpp"

		"src/builders/ModelLoader.cpp"

		"src/cleanup/DeletionQueue.cpp"
		"include/public/cleanup/Releasable.h"

		"src/validation/result.cpp"
		"src/validation/dispatch.cpp"

		"src/instance/CobaltInstance.cpp"
		"src/instance/VulkanInstance.cpp"
		"src/validation/compare.cpp"
		"src/assets/Window.cpp"
		include/public/event/MulticastDelegate.h
		src/instance/ValidationLayers.cpp
		include/public/instance/ValidationLayers.h
		src/validation/PhysicalDeviceSelector.cpp
		include/public/validation/PhysicalDeviceSelector.h
		src/query/queue_family.cpp
		include/private/query/queue_family.h
		include/private/query/swapchain_support.cpp
		include/private/query/swapchain_support.h
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
		PUBLIC "./include/public"
		PRIVATE "./include/private"
		PRIVATE "./src" )

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
