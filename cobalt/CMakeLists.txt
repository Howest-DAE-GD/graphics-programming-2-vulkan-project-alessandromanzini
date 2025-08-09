# VulkanEngine CMakeList.txt, "Author": alessandromanzini
# Engine specific logic here.
#
project("cobalt")

# create library target
add_library(${PROJECT_NAME}
		"src/__buffer/Buffer.cpp"

		"src/__cleanup/DeletionQueue.cpp"

		"src/CobaltVK.cpp"
		"src/__context/Window.cpp"

		"src/__buffer/CommandBuffer.cpp"
		"src/__buffer/CommandPool.cpp"
		"src/__buffer/CommandOperator.cpp"
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
		"src/__context/Queue.cpp"

		"include/public/__event/multicast_delegate/MulticastDelegate.h"
		"include/public/__event/multicast_delegate/Dispatcher.h"

		"src/__image/Image.cpp"
		"src/__image/ImageView.cpp"
		"src/__image/ImageLayoutTransition.cpp"
		"src/__image/TextureImage.cpp"
		"src/__image/StbImageLoader.cpp"

		"include/public/__init/InitWizard.h"

		"include/public/__memory/handle/ResourceHandle.h"
		"include/public/__memory/handle/handle_aliases.h"
		"include/public/__memory/memory_aliases.h"
		"include/public/__memory/Resource.h"
		"src/__memory/handle/HandleTable.cpp"

		"include/public/__meta/crtp.h"
		"include/public/__meta/function_traits.h"
		"include/public/__meta/vk_function_traits.h"
		"include/public/__meta/pack.h"
		"include/public/__meta/index_of.h"
		"include/public/__meta/enum_traits.h"
		"include/public/__meta/expect_size.h"

		"src/__model/Model.cpp"
		"src/__model/AssimpModelLoader.cpp"

		"src/__pipeline/GraphicsPipeline.cpp"
		"src/__pipeline/Pipeline.cpp"

		"src/__query/device_features_support.cpp"
		"src/__query/device_queries.cpp"
		"src/__query/extension_support.cpp"
		"src/__query/queue_family.cpp"
		"src/__query/swapchain_support.cpp"

		"include/public/__renderer/BindingDescription.h"
		"src/__renderer/DescriptorSetLayout.cpp"
		"src/__renderer/Renderer.cpp"
		"src/__renderer/DescriptorAllocator.cpp"
		"src/__renderer/WriteDescription.cpp"

		"src/__shader/ShaderModule.cpp"

		"src/__swapchain/Swapchain.cpp"

		"src/__synchronization/Semaphore.cpp"
		"src/__synchronization/SubmitInfo.cpp"

		"src/__validation/selector/PhysicalDeviceSelector.cpp"
		"src/__validation/result.cpp"
		"src/__validation/device_features.cpp"
		"src/__validation/dispatch.cpp"
		"src/__validation/compare.cpp"

		"src/log.cpp"
		src/__synchronization/PresentInfo.cpp
		include/public/__synchronization/PresentInfo.h
		src/__synchronization/Fence.cpp
		include/public/__synchronization/Fence.h
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
		PRIVATE assimp
		PRIVATE stb )

# require vulkan
include(vulkan_require)
