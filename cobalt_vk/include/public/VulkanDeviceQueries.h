#ifndef VULKANDEVICEQUERIES_H
#define VULKANDEVICEQUERIES_H

#include <optional>
#include <vector>
#include <vulkan/vulkan.h>


namespace cobalt_vk::query
{
    // +---------------------------+
    // | STRUCTS                   |
    // +---------------------------+






    // +---------------------------+
    // | MEMORY                    |
    // +---------------------------+
    [[nodiscard]] uint32_t find_memory_type( VkPhysicalDevice physicalDevice, uint32_t typeFilter,
                                             VkMemoryPropertyFlags properties );

    // +---------------------------+
    // | FORMAT                    |
    // +---------------------------+
    [[nodiscard]] VkFormat find_supported_format( VkPhysicalDevice physicalDevice,
                                                  const std::vector<VkFormat>& candidates,
                                                  VkImageTiling tiling, VkFormatFeatureFlags features );
    [[nodiscard]] VkFormat find_depth_format( VkPhysicalDevice physicalDevice );
    [[nodiscard]] bool has_stencil_component( VkFormat format );
}

#endif //VULKANDEVICEQUERIES_H
