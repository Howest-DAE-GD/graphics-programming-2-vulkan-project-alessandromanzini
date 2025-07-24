# VulkanEngine CMakeList.txt, "Author": alessandromanzini
# Engine specific logic here.
#
project("cobalt")

# create library target
add_library(${PROJECT_NAME}
        src/VulkanDeviceQueries.cpp
		src/ShaderModules.cpp
		src/UniformBufferObject.cpp
		include/public/UniformBufferObject.h
		include/public/SingleTimeCommand.h

		"src/assets/Buffer.cpp"
		"src/assets/Model.cpp"

		"src/cleanup/DeletionQueue.cpp"

		"src/CobaltVK.cpp"
		"src/assets/Window.cpp"

		"src/__buffer/FrameBuffer.cpp"

		"include/public/__builder/VkBuilder.h"
		"src/__builder/ModelLoader.cpp"
		"src/__builder/VkSwapchainBuilder.cpp"
		"src/__builder/PhysicalDeviceSelector.cpp"

		"src/__context/DeviceSet.cpp"
		"src/__context/InstanceBundle.cpp"
		"src/__context/VkContext.cpp"
		"src/__context/ValidationLayers.cpp"

		"include/public/__event/multicast_delegate/MulticastDelegate.h"
		"include/public/__event/multicast_delegate/Dispatcher.h"

		"src/__image/Image.cpp"
		"src/__image/ImageView.cpp"

		"include/public/__memory/handle/ResourceHandle.h"
		"include/public/__memory/handle/handle_aliases.h"
		"include/public/__memory/memory_aliases.h"
		"include/public/__memory/Resource.h"
		"src/__memory/handle/HandleTable.cpp"

		"include/public/__meta/crtp.h"
		"include/public/__meta/function_traits.h"
		"include/public/__meta/vk_function_traits.h"

		"src/__swapchain/Swapchain.cpp"

		"src/__validation/result.cpp"
		"src/__validation/dispatch.cpp"
		"src/__validation/compare.cpp"

		"src/__query/extension_support.cpp"
		"src/__query/queue_family.cpp"
		"src/__query/swapchain_support.cpp"

		"src/log.cpp"
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
