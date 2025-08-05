# VulkanEngine CMakeList.txt, "Author": alessandromanzini
# Engine specific logic here.
#
project("cobalt")

# create library target
add_library(${PROJECT_NAME}
		src/UniformBufferObject.cpp
		include/public/UniformBufferObject.h
		include/public/SingleTimeCommand.h

		"src/assets/Buffer.cpp"

		"src/cleanup/DeletionQueue.cpp"

		"src/CobaltVK.cpp"
		"src/assets/Window.cpp"

		"src/__buffer/Framebuffer.cpp"

		"include/private/__builder/VkBuilder.h"
		"src/__builder/VkSwapchainBuilder.cpp"

		"include/public/__command/Command.h"
		"include/public/__command/ValidationCommandBind.h"
		"src/__command/ValidationCommand.cpp"

		"src/__context/DeviceSet.cpp"
		"src/__context/InstanceBundle.cpp"
		"src/__context/VkContext.cpp"
		"src/__context/ValidationLayers.cpp"

		"src/__enum/DeviceFeatureFlags.cpp"

		"include/public/__event/multicast_delegate/MulticastDelegate.h"
		"include/public/__event/multicast_delegate/Dispatcher.h"

		"src/__image/Image.cpp"
		"src/__image/ImageView.cpp"

		"include/public/__init/InitWizard.h"

		"include/public/__memory/handle/ResourceHandle.h"
		"include/public/__memory/handle/handle_aliases.h"
		"include/public/__memory/memory_aliases.h"
		"include/public/__memory/Resource.h"
		"src/__memory/handle/HandleTable.cpp"

		"src/__model/Model.cpp"
		"src/__model/ModelLoader.cpp"
		"src/__model/AssimpModelLoader.cpp"

		"src/__renderer/Renderer.cpp"

		"src/__shader/ShaderModule.cpp"

		"include/public/__meta/crtp.h"
		"include/public/__meta/function_traits.h"
		"include/public/__meta/vk_function_traits.h"
		"include/public/__meta/pack.h"
		"include/public/__meta/index_of.h"

		"src/__swapchain/Swapchain.cpp"

		"src/__validation/result.cpp"
		"src/__validation/device_features.cpp"
		"src/__validation/dispatch.cpp"
		"src/__validation/compare.cpp"
		"src/__validation/selector/PhysicalDeviceSelector.cpp"

		"src/__query/device_features_support.cpp"
		"src/__query/extension_support.cpp"
		"src/__query/queue_family.cpp"
		"src/__query/swapchain_support.cpp"

		"src/log.cpp"
		src/__pipeline/GraphicsPipeline.cpp
        include/public/__pipeline/GraphicsPipeline.h
		include/public/__renderer/BindingDescription.h
		src/__renderer/DescriptorSetLayout.cpp
		include/public/__renderer/DescriptorSetLayout.h
		src/__buffer/CommandBuffer.cpp
		include/public/__buffer/CommandBuffer.h
		src/__buffer/CommandPool.cpp
		include/public/__buffer/CommandPool.h
		src/__pipeline/Pipeline.cpp
		include/public/__pipeline/Pipeline.h
		include/public/__buffer/CommandOperator.cpp
		include/public/__buffer/CommandOperator.h
		include/private/__query/device_queries.h
		src/__query/device_queries.cpp
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
