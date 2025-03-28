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