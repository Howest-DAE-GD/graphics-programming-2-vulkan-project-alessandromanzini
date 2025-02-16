# VulkanEngine CMakeList.txt, "Author": Manzini Alessandro
# Engine specific logic here.
#
project("VulkanEngine")

# create library target
add_library("${PROJECT_NAME}"
	"src/test.cpp" )
		
# the include folder and resources has headers that will be shared to external project.
# the src folder also need to be included in case we have 
# headers for internal use.
target_include_directories("${PROJECT_NAME}"
		PUBLIC "./include"
		PRIVATE "./src" )

# fetch libraries
include(glfw_fetchcontent)
include(glm_fetchcontent)

target_link_libraries("${PROJECT_NAME}"
		PUBLIC glfw 
		PUBLIC glm )

# require vulkan
find_package(Vulkan REQUIRED)

if (APPLE)
	# Use MoltenVK
	set(MOLTEN_VK_LIB "$ENV{VULKAN_SDK}/lib/libMoltenVK.dylib")

	add_definitions(-DVK_USE_PLATFORM_METAL_EXT)
	target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan ${MOLTEN_VK_LIB})
	message(STATUS "Vulkan SDK loaded with MoltenVK")
else()
	target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan)
	message(STATUS "Vulkan SDK loaded")
endif()
